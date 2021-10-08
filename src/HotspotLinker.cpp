/*
 * HotspotLinker.cpp
 *
 *  Created on: 2016/01/05
 *      Author: PC-EFFECT-012
 */

#include <stdio.h>
#include "HotSpotLinker.h"
#include "app.h"

//------------------------------------------------------------------------------
// Macro
//------------------------------------------------------------------------------

#define unsetHotspotId(m, n)\
	setHotspotId((m), (n), ResultData::NOT_ASSIGNED)

// foreach構文
#define __foreach(item_type, value, list) \
	for (\
		std::vector<item_type>::iterator \
		value = list.begin(); \
		value != list.end(); \
		value++\
	)

//------------------------------------------------------------------------------
// Inline Functions
//------------------------------------------------------------------------------

inline double getRelativeRatio(ResultData *result, int picNo1, int picNo2) {
	const double height1 = result->getHeight(picNo1);
	const double height2 = result->getHeight(picNo2);
	if (height1 * height2 == 0) {
		return 1.0;
	}

	const double size = result->getInfraredWidth();
	const double viewAngle = result->getViewAngle();

	double mpp1 = ResultData::getMeterPerPixel(size, viewAngle, height1);
	double mpp2 = ResultData::getMeterPerPixel(size, viewAngle, height2);

	return mpp2 / mpp1;
}

//------------------------------------------------------------------------------
// Class Functions
//------------------------------------------------------------------------------

HotspotLinker::HotspotLinker(ResultData *result) :
		result(result) {
	links.clear();
}

HotspotLinker::~HotspotLinker(void) {
}

/**
 * 指定したリンクIDが指定した画像内に存在するかどうか確認する
 * @param id 確認するリンクID
 * @param pictureNo 確認する画像番号
 * @return true:存在する/false:存在しない
 */
bool HotspotLinker::existsId(int id, int pictureNo) {
	// idが範囲外の場合
	if ((id < 0) || (id >= (int) links.size())) {
		return false;
	}
	// pictureNoが範囲外の場合
	if ((pictureNo < 0) || (pictureNo >= result->getDataCount())) {
		return false;
	}

	const int count = result->getHotspotCount(pictureNo);

	// idがpictureNoの画像にないかチェックし、ある場合はリンクしない
	for (int i = 0; i < count; i++) {
		if (id == result->getHotspotId(pictureNo, i)) {
			// pictureNoの画像にidのホットスポットが既にある場合
			return true;
		}
	}
	return false;
}

/**
 * ホットスポットリンクを追加する
 * @param picNo1 一つ目のホットスポットの画像番号
 * @param ptNo1 一つ目のホットスポットの画像内番号
 * @param picNo2 ふたつ目のホットスポットの画像番号
 * @param ptNo2 ふたつ目のホットスポットの画像内番号
 * @return ホットスポットリンク番号(固有番号)
 */
int HotspotLinker::add(int picNo1, int ptNo1, int picNo2, int ptNo2) {
	int id1 = ResultData::NOT_ASSIGNED, id2 = ResultData::NOT_ASSIGNED;

	// 存在確認
	if ((picNo1 == picNo2) || (!exists(picNo1, ptNo1))
			|| (!exists(picNo2, ptNo2))) {
		return ResultData::NOT_ASSIGNED;
	}

	id1 = result->getHotspotId(picNo1, ptNo1);
	// id1がpicNo2の画像にないかチェックし、ある場合はリンクしない
	if (id1 != ResultData::NOT_ASSIGNED) {
		if (existsId(id1, picNo2)) {
			return ResultData::NOT_ASSIGNED;
		}
	}

	id2 = result->getHotspotId(picNo2, ptNo2);
	if (id2 != ResultData::NOT_ASSIGNED) {
		// id2がpicNo1の画像にないかチェックし、ある場合はリンクしない
		if (existsId(id2, picNo1)) {
			return ResultData::NOT_ASSIGNED;
		}
	}

	// 同じタイプのホットスポット間のみリンクできる
	if (!compareType(picNo1, ptNo1, picNo2, ptNo2)) {
		return ResultData::NOT_ASSIGNED;
	}

	int id = 0;
	if ((id1 == ResultData::NOT_ASSIGNED)
			&& (id2 == ResultData::NOT_ASSIGNED)) {
		// どちらも固有番号が割り当てられていない場合
		id = (int) links.size();
		HotspotLink link;
		HotspotNumber num1, num2;
		num1.pictureNo = picNo1;
		num1.pointNo = ptNo1;
		num2.pictureNo = picNo2;
		num2.pointNo = ptNo2;
		link.push_back(num1);
		link.push_back(num2);
		links.push_back(link);
		result->setHotspotId(picNo1, ptNo1, id);
		result->setHotspotId(picNo2, ptNo2, id);
	} else if ((id1 != ResultData::NOT_ASSIGNED)
			&& (id2 == ResultData::NOT_ASSIGNED)) {
		// 1つめのみ設定されていた場合
		// 2つめに1番のIDを割り当てる
		id = add(id1, picNo2, ptNo2);
	} else if ((id1 == ResultData::NOT_ASSIGNED)
			&& (id2 != ResultData::NOT_ASSIGNED)) {
		// 2つめのみ割り当てられていた場合
		// 1つめに2番のIDを割り当てる
		id = add(id2, picNo1, ptNo1);
	} else {
		// 両方とも割り当てられていた場合
		// 両方に同じIDを割り当てる
		id = merge(id1, id2);
	}

	if (id != ResultData::NOT_ASSIGNED) {
		autoLink(picNo1, ptNo1, picNo2, ptNo2);
	}

	return id;
}

/**
 * ホットスポットリンクを追加する
 * @param id ホットスポット固有番号(ID)
 * @param picNo 画像番号
 * @param ptNo ホットスポットの画像内番号
 * @return ホットスポットリンク番号(固有番号)
 */
int HotspotLinker::add(int id, int picNo, int ptNo) {
	HotspotNumber num;
	num.pictureNo = picNo;
	num.pointNo = ptNo;
	links[id].push_back(num);
	result->setHotspotId(picNo, ptNo, id);
	return id;
}

void HotspotLinker::remove(int id) {
	// 指定したIDに割り当てられているホットスポットをすべて未割り当てにする
	__foreach (HotspotNumber, num, links[id])
	{
		result->unsetHotspotId(num->pictureNo, num->pointNo);
	}
	links[id].clear();
	HotspotLink().swap(links[id]);

	// 指定したID以降のIDをひとつ前にずらす
	int count = (int) links.size();
	for (int i = id + 1; i < count; i++) {
		__foreach (HotspotNumber, num, links[i])
		{
			result->setHotspotId(num->pictureNo, num->pointNo, i - 1);
		}
	}

	// 指定したIDをリストから削除する
	links.erase(links.begin() + id);
}

void HotspotLinker::remove(int picNo, int ptNo) {
	int id = result->getHotspotId(picNo, ptNo);

	if ((id < 0) || id >= (int) links.size()) {
		return;
	}

	// 指定したホットスポットを未割り当てにする
	result->unsetHotspotId(picNo, ptNo);

	// リンクリストから削除する
	__foreach (HotspotNumber, num, links[id])
	{
		if ((picNo == num->pictureNo) && (ptNo == num->pointNo)) {
			links[id].erase(num);
			break;
		}
	}

	// リンクリストが空になった場合はIDをリストから削除する
	if (links[id].size() <= 1) {
		remove(id);
	}
}

int HotspotLinker::merge(int id1, int id2) {
	// 引数チェック
	if ((id1 < 0 || id1 >= (int) links.size())
			|| (id2 < 0 || id2 >= (int) links.size())) {
		// どちらかが範囲外の場合は-1を返す
		return ResultData::NOT_ASSIGNED;
	}
	if (id1 == id2) {
		// 両方とも同じ番号の場合は何もしない
		return id1;
	}

	// 重複リンクのチェック
	__foreach (HotspotNumber, item1, links[id1])
	{
		__foreach(HotspotNumber, item2, links[id2])
		{
			if (item1->pictureNo == item2->pictureNo) {
				// 両方の画像に同じIDがある場合は-1を返す
				return ResultData::NOT_ASSIGNED;
			}
		}
	}

	// 小さいほうの番号を使用する
	int idMax, idMin;

	if (id1 > id2) {
		idMax = id1;
		idMin = id2;
	} else {
		idMax = id2;
		idMin = id1;
	}

	// 大きいほうのホットスポットのリストを取得する
	std::vector<HotspotNumber> list = (*this)[idMax];

	// 大きいほうのIDは削除する
	remove(idMax);

	// 大きいほうに割り当てられていたIDを小さいほうに割り当てる
	__foreach (HotspotNumber, item, list)
	{
		add(idMin, item->pictureNo, item->pointNo);
	}

	// 小さいほうのIDを返す
	return idMin;
}

HotspotLink HotspotLinker::operator[](int id) {
	return links[id];
}

bool HotspotLinker::exists(int picNo, int ptNo) {
	// 画像番号が適切か確認
	if ((picNo < 0) || (picNo >= (int) result->getDataCount())) {
		return false;
	}
	// スポット番号が適切か確認
	if ((ptNo < 0) || (ptNo >= (int) result->getHotspotCount(picNo))) {
		return false;
	}
	// 存在している
	return true;
}

bool HotspotLinker::compareType(int picNo1, int ptNo1, int picNo2, int ptNo2) {
	int type[2] = { result->getHotspotType(picNo1, ptNo1),
			result->getHotspotType(picNo2, ptNo2) };

	for (int i = 0; i < 2; i++) {
		switch (type[i]) {
		case HOTSPOT_TYPE_CANDIDATE:
		case HOTSPOT_TYPE_ENABLED:
			type[i] = HOTSPOT_TYPE_ENABLED;
			break;
		}
	}

	// 両方とも同じタイプだった場合
	if (type[0] == type[1]) {
		return true;
	}

	return false;
}

void HotspotLinker::autoLink(int picNo1, int ptNo1, int picNo2, int ptNo2) {
	double ratio = getRelativeRatio(result, picNo1, picNo2);

	Vector2D d0;
	result->getHotspotRelativePos(picNo1, ptNo1, picNo2, ptNo2, ratio, &d0);

	for (int i = 0; i < result->getHotspotCount(picNo1); i++) {
		if ((ptNo1 == i) || (result->isUniqueHotspot(picNo1, i) == false)) {
			// 同じ番号のものは無視する
			continue;
		}
		for (int j = 0; j < result->getHotspotCount(picNo2); j++) {
			if ((ptNo2 == j) || (result->isUniqueHotspot(picNo2, j) == false)) {
				// 同じ番号のものは無視する
				continue;
			}

			Vector2D d1;
			result->getHotspotRelativePos(picNo1, i, picNo2, j, ratio, &d1);

			double dx = d1.x - d0.x;
			double dy = d1.y - d0.y;

			if (sqrt(dx * dx + dy * dy) < 40) {
				// 無効なホットスポット同士の場合は一時的に有効にする
				bool enabled1 = false;
				if (result->isDisabledHotspot(picNo1, i)) {
					result->enableHotspot(picNo1, i);
					enabled1 = true;
				}
				bool enabled2 = false;
				if (result->isDisabledHotspot(picNo2, j)) {
					result->enableHotspot(picNo2, j);
					enabled2 = true;
				}
				// 自動的にリンクする
				if (add(picNo1, i, picNo2, j) == ResultData::NOT_ASSIGNED) {
					// リンク失敗
					if (enabled1 == true) {
						result->disableHotspot(picNo1, i);
					}
					if (enabled2 == true) {
						result->disableHotspot(picNo2, j);
					}
				}
				break;
			}
		}
	}
}

bool HotspotLinker::saveTo(const TCHAR *path) {
	// データをファイルに保存する
	FILE *file = fileOpen(path, TEXT("w"));
	if (file == NULL) {
		return false;
	}
	for (int i = 0; i < (int) links.size(); i++) {
		__foreach (HotspotNumber, number, links[i])
		{
			fprintf(file, "%d,%d,%d\n", i, number->pictureNo, number->pointNo);
		}
	}
	fclose(file);
	return false;
}

bool HotspotLinker::loadFrom(const TCHAR *path) {
	// ファイルを開く
	FILE *file = fileOpen(path, TEXT("r"));
	if (file == NULL) {
		return false;
	}
	links.clear();
	while (!feof(file)) {
		int i;
		HotspotNumber num;
		int ret = fscanf(file, "%d,%d,%d\n", &i, &num.pictureNo, &num.pointNo);
		if (ret == 3) {
			// 画像番号が適切か確認
			if (!exists(num.pictureNo, num.pointNo)) {
				continue;
			}
			links.resize(i + 1);
			add(i, num.pictureNo, num.pointNo);
		}
	}
	fclose(file);
	return true;
}

void HotspotLinker::removeHotspot(int picNo, int ptNo) {
	// リンクを削除する
	remove(picNo, ptNo);

	// 同一画像上のホットスポットが含まれるすべてのリンクを調べ、
	// ptNoに指定した番号以降のホットスポットを1つ前にずらす
	__foreach (HotspotLink, link, links)
	{
		__foreach(HotspotNumber, num, (*link))
		{
			if ((num->pictureNo == picNo) && (num->pointNo > ptNo)) {
				num->pointNo--;
			}
		}
	}

	// ホットスポットを削除する
	result->delHotspot(picNo, ptNo);
}

void HotspotLinker::clear(void) {
	__foreach (HotspotLink, link, links)
	{
		link->clear();
	}
	links.clear();
}

/**
 * 総リンクの数を取得する
 * @return 総リンク数
 */
int HotspotLinker::linkCount(void) {
	return links.size();
}

/**
 * ホットスポットのリンクの数を取得する
 * @return ホットスポットのリンク数
 */
int HotspotLinker::hotspotCount(void) {
	int linkSize = 0;
	__foreach (HotspotLink, link, links)
	{
		int picNo = link->at(0).pictureNo;
		int ptNo = (*link)[0].pointNo;
		if (result->isHotspot(picNo, ptNo)) {
			linkSize++;
		}
	}
	return linkSize;
}

/**
 * ホットスポットリンクが作成可能か確認する。ただしタイプの確認はしない。
 * @param picNo1 一つ目のホットスポットの画像番号
 * @param ptNo1 一つ目のホットスポットの画像内番号
 * @param picNo2 ふたつ目のホットスポットの画像番号
 * @param ptNo2 ふたつ目のホットスポットの画像内番号
 * @return true:作成可能/false:作成不可
 */
bool HotspotLinker::testWithoutType(int picNo1, int ptNo1, int picNo2,
		int ptNo2) {
	int id1 = ResultData::NOT_ASSIGNED, id2 = ResultData::NOT_ASSIGNED;

	// 存在確認
	if ((picNo1 == picNo2) || (!exists(picNo1, ptNo1))
			|| (!exists(picNo2, ptNo2))) {
		return false;
	}

	id1 = result->getHotspotId(picNo1, ptNo1);
	// id1がpicNo2の画像にないかチェックし、ある場合はリンクしない
	if (id1 != ResultData::NOT_ASSIGNED) {
		if (existsId(id1, picNo2)) {
			return false;
		}
	}

	id2 = result->getHotspotId(picNo2, ptNo2);
	if (id2 != ResultData::NOT_ASSIGNED) {
		// id2がpicNo1の画像にないかチェックし、ある場合はリンクしない
		if (existsId(id2, picNo1)) {
			return false;
		}
	}

	if ((id1 != ResultData::NOT_ASSIGNED)
			&& (id2 != ResultData::NOT_ASSIGNED)) {
		// 両方とも割り当てられていた場合
		// 重複リンクのチェック
		__foreach (HotspotNumber, item1, links[id1])
		{
			__foreach(HotspotNumber, item2, links[id2])
			{
				if (item1->pictureNo == item2->pictureNo) {
					// 両方の画像に同じIDがある場合は-1を返す
					return false;
				}
			}
		}
	}

	return true;
}

//　(2017/6/14YM)リンクデータ変数クリアチェック関数追加
void HotspotLinker::CheckLinkClearData(void) {
	//　ホットスポットデータクリアチェック
	if (links.size() == 0) {
		links.clear();
		std::vector<HotspotLink>().swap(links);
	}
}

#include <queue>
bool HotspotLinker::checkAllLinked(void) {
	bool *checked = this->linkedPictures(0);
	if (checked != NULL) {
		const int DATA_COUNT = result->getDataCount();
		int linkCount = 0;
		for (int i = 0; i < DATA_COUNT; i++) {
			if (checked[i] == true) {
				linkCount++;
			}
		}
		if (linkCount == DATA_COUNT) {
			return true;
		}
		delete checked;
	}
	return false;
}

// (2017/9/4YM)リンクされていない画像を検出する関数を追加
bool* HotspotLinker::checkAllLinkedData(void) {

	const int DATA_COUNT = result->getDataCount();

	int start = 0;

	bool *checked = new bool[DATA_COUNT];
	ZeroMemory(checked, DATA_COUNT * sizeof(bool));
	if (links.size() > 0) {
		std::queue<int> queue;
		queue.push(start);
		checked[start] = true;

		do {
			int picNo = queue.front();
			queue.pop();
			const int COUNT = result->getHotspotCount(picNo);
			for (int i = 0; i < COUNT; i++) {
				int id = result->getHotspotId(picNo, i);
				if (id != ResultData::NOT_ASSIGNED) {
					__foreach (HotspotLink, link, links)
					{
						__foreach (HotspotNumber, num, (*link))
						{
							if (checked[num->pictureNo] == false) {
								queue.push(num->pictureNo);
								checked[num->pictureNo] = true;
							}
						}
					}
				}
			}
		} while (!queue.empty());
	}

	return checked;
}

bool* HotspotLinker::linkedPictures(int start) {
	const int DATA_COUNT = result->getDataCount();
	if ((start < 0) || (start >= DATA_COUNT)) {
		return NULL;
	}
	bool *checked = new bool[DATA_COUNT];
	ZeroMemory(checked, DATA_COUNT * sizeof(bool));

	// (2017/6/20YM)リンク数チェックを追加
	if (links.size() > 0) {
		std::queue<int> queue;
		queue.push(start);
		checked[start] = true;
		do {
			int picNo = queue.front();
			queue.pop();
			const int COUNT = result->getHotspotCount(picNo);
			for (int i = 0; i < COUNT; i++) {
				int id = result->getHotspotId(picNo, i);
				if (id != ResultData::NOT_ASSIGNED) {
					__foreach (HotspotLink, link, links)
					{
						__foreach (HotspotNumber, num, (*link))
						{
							if (checked[num->pictureNo] == false) {
								queue.push(num->pictureNo);
								checked[num->pictureNo] = true;
							}
						}
					}
				}
			}
		} while (!queue.empty());
	}

	return checked;
}
