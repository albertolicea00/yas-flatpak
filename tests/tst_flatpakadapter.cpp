#include <QTest>

#include "flatpakadapter.h"

class TestFlatpakAdapter : public QObject {
    Q_OBJECT
private slots:
    void searchParsesTabColumns()
    {
        FlatpakAdapter adapter;
        const auto packages = adapter.parseSearch(QStringLiteral(
            "org.videolan.VLC\tVLC\t3.0.20\tVLC media player\tflathub\n"
            "org.gimp.GIMP\tGIMP\t2.10.38\tImage editor\tflathub\n"));
        QCOMPARE(packages.size(), 2);
        QCOMPARE(packages.at(0).id, QStringLiteral("org.videolan.VLC"));
        QCOMPARE(packages.at(0).name, QStringLiteral("VLC"));
        QCOMPARE(packages.at(0).source, QStringLiteral("flathub"));
    }

    void installedParsesTabColumns()
    {
        FlatpakAdapter adapter;
        const auto packages = adapter.parseInstalled(QStringLiteral(
            "org.videolan.VLC\tVLC\t3.0.20\tflathub\tVLC media player\n"));
        QCOMPARE(packages.size(), 1);
        QCOMPARE(packages.at(0).installedVersion, QStringLiteral("3.0.20"));
        QVERIFY(packages.at(0).installed());
    }

    void maskIsPinEquivalent()
    {
        FlatpakAdapter adapter;
        QCOMPARE(adapter.pinCommand("org.videolan.VLC", "").arguments,
                 QStringList({"mask", "org.videolan.VLC"}));
        QCOMPARE(adapter.unpinCommand("org.videolan.VLC", "").arguments,
                 QStringList({"mask", "--remove", "org.videolan.VLC"}));
    }

    void mutationsRunUnprivileged()
    {
        FlatpakAdapter adapter;
        // flatpak handles polkit itself — no pkexec wrapper anywhere.
        QCOMPARE(adapter.installCommand("x", "").program, QStringLiteral("flatpak"));
        QVERIFY(adapter.installCommand("x", "").arguments.contains(
            QStringLiteral("--noninteractive")));
    }
};

QTEST_MAIN(TestFlatpakAdapter)
#include "tst_flatpakadapter.moc"
