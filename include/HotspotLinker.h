/*
 * HotspotLinker.h
 *
 *  Created on: 2016/02/01
 *      Author: PC-EFFECT-011
 */

//　(2017/6/14YM)リンクデータ変数クリアチェック関数追加
//　(2017/9/4YM)リンクデータ取得関数追加
#ifndef HOTSPOTLINKER_H_
#define HOTSPOTLINKER_H_

#include "ResultData.h"

/**
 * ホットスポットの連結情報生成器
 */
class HotspotLinker {
public:
	HotspotLinker(ResultData *result);
	~HotspotLinker(void);

	int add(int picNo1, int ptNo1, int picNo2, int ptNo2);
	void remove(int id);
	void remove(int picNo, int ptNo);
	int merge(int id1, int id2);
	HotspotLink operator[](int id);

	void autoLink(int picNo1, int ptNo1, int picNo2, int ptNo2);

	void clear(void);

	bool saveTo(const TCHAR *path);
	bool loadFrom(const TCHAR *path);

	void removeHotspot(int picNo, int ptNo);
	int hotspotCount(void);

	int linkCount(void);

	bool checkAllLinked(void);
	bool* linkedPictures(int start);

	bool existsId(int id, int pictureNo);

	bool testWithoutType(int picNo1, int ptNo1, int picNo2, int ptNo2);

	//　(2017/6/14YM)リンクデータ変数クリアチェック関数追加
	void CheckLinkClearData(void);

	//　(2017/9/4YM)リンクデータ取得関数追加
	bool* checkAllLinkedData(void);

protected:
	bool exists(int picNo, int ptNo);
	bool compareType(int picNo1, int ptNo1, int picNo2, int ptNo2);

private:
	int add(int id, int picNo, int ptNo);

	ResultData *result;
	std::vector<HotspotLink> links;
};

#endif /* HOTSPOTLINKER_H_ */
