#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "appcontroller.h"
#include "bootstrap.h"
#include "flatpakadapter.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("YAS"));
    app.setApplicationName(QStringLiteral("yas-flatpak"));
    app.setApplicationDisplayName(QStringLiteral("Yet Another Store for Flatpak"));

    QQuickStyle::setStyle(QStringLiteral("Basic"));
    yas::loadBundledFonts();

    FlatpakAdapter adapter;
    yas::AppController controller(&adapter);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("App"), &controller);
    engine.loadFromModule("YasFlatpak", "Main");
    return engine.rootObjects().isEmpty() ? 1 : app.exec();
}
