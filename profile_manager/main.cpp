#include "profilesmanagerdialog.h"

#include <QApplication>
#include <QStyleFactory>

#include "singleapplication/singleapplication.h"

using namespace arcirk::profile_manager;

int main(int argc, char *argv[])
{
    SingleApplication app( argc, argv );
#ifndef DEBUG
    app.setQuitOnLastWindowClosed(false);
#endif
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QStringList cmdline_args = QCoreApplication::arguments();

    bool bHidden = cmdline_args.indexOf("-h") != -1;
    ProfilesManagerDialog w;
    if(bHidden)
        w.hide();
    else
        w.show();
    return app.exec();

}
