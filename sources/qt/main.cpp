#include <QApplication>
#include "REMainWindow.h"
#include "RESplash.h"

#include <RESong.h>
#include <REScoreSettings.h>
#include <REScore.h>
#include <REGuitarProParser.h>
#include <REInputStream.h>
#include <REPainter.h>
#include <RESoundFontManager.h>
#include <RERtAudioEngine.h>
#include <REJackAudioEngine.h>

#include "REDocumentView.h"

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QImageWriter>
#include <QPainter>
#include <QMessageBox>
#include <QLabel>
#include <QScrollArea>
#include <QTabWidget>


#ifdef MACOSX
#  include <CoreFoundation/CoreFoundation.h>
#endif

std::string BundlePath()
{
#if defined(WIN32)
#  ifdef DEBUG
    return "..";
#  else
    return ".";
#  endif
#elif defined(LINUX)
    return ".";
#elif defined(MACOSX)
    char path[1024];
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    assert( mainBundle );

    CFURLRef mainBundleURL = CFBundleCopyBundleURL( mainBundle);
    assert( mainBundleURL);

    CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
    assert( cfStringRef);

    CFStringGetCString( cfStringRef, path, 1024, kCFStringEncodingASCII);

    CFRelease( mainBundleURL);
    CFRelease( cfStringRef);

    return std::string( path);
#endif
}

std::string DataPath()
{
#ifdef MACOSX
    return BundlePath() + "/Contents/Resources" ;
#else
    return BundlePath() + "/data";
#endif
}

void LoadGP(RESong& song, QString filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    QByteArray bytes = file.readAll();
    file.close();
    REBufferInputStream decoder(bytes.data(), bytes.size());
    decoder.SetVersion(REFLOW_IO_VERSION);
    REGuitarProParser parser;
    parser.Parse(&decoder, &song);
}



QPixmap TestRenderOnePage(const REScore& score, QString filename)
{
    RERect rc = score.ContentRect();
    QImage img(rc.size.w, rc.size.h, QImage::Format_ARGB32);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // Draw page here
    REPainter rp(&painter);
    rp.SetFillColor(REColor::White);
    rp.FillRect(rc);
    score.DrawPage(rp, 1);
    
    QImageWriter writer(filename);
    writer.write(img);
    
    return QPixmap::fromImage(img);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("Gargant Studios");
    a.setOrganizationDomain("gargant.com");
    a.setApplicationName("Reflow");

    // Splash screen
    RESplash splash;
    splash.show();

    // Default soundfont path
    RESoundFontManager::Instance().SetDefaultSoundFontPath(DataPath() + "/GeneralUser.sf2");
    
    REAudioSettings settings;
    RERtAudioEngine* audio = RERtAudioEngine::Instance();
    //REJackAudioEngine* audio = REJackAudioEngine::Instance();
	audio->Initialize(settings);
	audio->StartRendering();
    
	REMainWindow w;
    w.ActionNew();
    w.show();

    return a.exec();
}
