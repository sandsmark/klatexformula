
#include <QDebug>
#include <QList>
#include <QString>
#include <QStringList>
#include <QByteArray>

#include <windows.h>


void klfWinClipboardCopy(HWND h, const QStringList& wintypes, const QList<QByteArray>& datalist)
{
  if (wintypes.size() != datalist.size()) {
    qWarning("win: Need same list sizes for wintypes and datalist! (got %d and %d resp.)",
	     (int)wintypes.size(), (int)datalist.size());
    return;
  }
  if ( ! OpenClipboard(h) ) {
    qWarning("win: Cannot open the Clipboard");
    return;
  }
  // Remove the current Clipboard contents
  if( ! EmptyClipboard() ) {
    qWarning("win: Cannot empty the Clipboard");
    return;
  }
  int k;
  for (k = 0; k < wintypes.size(); ++k) {
    const QString type = wintypes[k];
    const char * data = datalist[k].constData();
    const int size = datalist[k].size();
    // Register required datatype
    int wintype =
      RegisterClipboardFormatA(type.toLocal8Bit());
    if (!wintype) {
      qWarning("win: Unable to register clipboard format `%s' !", qPrintable(type));
      continue;
    }
    // Get the currently selected data
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, size);
    if ( ! hGlob ) {
      qWarning("win: Unable to GlobalAlloc(): %d", (int)GetLastError());
      continue;
    }
    char * ptr = (char*)GlobalLock(hGlob);
    if (ptr == NULL) {
      qWarning("win: Unable to GlobalLock(): %d", (int)GetLastError());
      continue;
    }
    memcpy(ptr, data, size);
    GlobalUnlock(hGlob);
    // Set clipboard data for this type
    if ( ::SetClipboardData( wintype, hGlob ) == NULL ) {
      qWarning("win: Unable to set Clipboard data, error: %d", (int)GetLastError());
      CloseClipboard();
      GlobalFree(hGlob);
      return;
    }
  }
  // finally close clipboard, we're done.
  CloseClipboard();
}