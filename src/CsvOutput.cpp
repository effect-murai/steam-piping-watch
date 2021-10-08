#include<fstream>
#include<iostream>

#include "app.h"
#include "ResultData.h"
#include "PanelInfo.h"
#include "StringUtils.h"
#include "FileUtils.h"

using namespace std;

extern ResultData *result;
extern PanelData *panelData;

#ifdef ShellExecute
#define shellOpen(file, parameters, workDir, showCommand)\
	((INT_PTR) ShellExecute(NULL, TEXT("open"), (file), (parameters), (workDir), (showCommand)))
#endif

/**
 * CSVファイルを出力する
 * @param path ファイル出力先
 * @return bool
 */
bool exportData(const TCHAR *path) {
	//　ファイルを開く
	FILE *file = fileOpen(path, TEXT("w"));
	if (file == NULL) {
		return false;
	}

	int max = panelData->getPanelNameCountMax();
	char *utf8String;

	// 開始日付出力
	utf8String = toUTF8String(result->getFirstDateString());
	fprintf(file, "%s\n", utf8String);
	delete utf8String;

	//　終了日付出力
	utf8String = toUTF8String(result->getLastDateString());
	fprintf(file, "%s\n", utf8String);
	delete utf8String;

	//　パネルごとのホットスポット総数を出力（パネル名、総数）
	for (int i = 0; i < max; i++) {
		utf8String = toUTF8String(panelData->getPanelName(i));
		// ファイルに書き込む
		fprintf(file, "%s,", utf8String);
		fprintf(file, "%d\n", panelData->getHotspotPanelCount(i));
		delete utf8String;
	}
	//　ファイルを閉じる
	fclose(file);

	return true;
}

bool openReport(TCHAR *fileName) {
#ifdef ShellExecute
	TCHAR *workingDirectory = toBaseName(clone(fileName));
	INT_PTR ret = shellOpen(fileName, NULL, workingDirectory, SW_MAXIMIZE);
	delete workingDirectory;
	if (ret <= 32) {
		return false;
	}
#else
	// 報告書の起動
	FILE* fp = popen("cmd", "w");
	fprintf(fp, "%S\n", fileName);
	pclose(fp);
#endif
	return true;
}
