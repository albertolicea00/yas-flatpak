#include "flatpakadapter.h"

using yas::CliAction;
using yas::CliCommand;
using yas::Package;

// Flatpak adapter. Read commands use --columns= (stable tab-separated
// output). No pkexec needed: flatpak triggers polkit itself for system
// installations.
namespace {

const QString kFlatpak = QStringLiteral("flatpak");

QList<QStringList> tabRows(const QString &stdOut)
{
    QList<QStringList> rows;
    const QStringList lines = stdOut.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (!line.contains(QLatin1Char('\t')))
            continue;
        rows.append(line.split(QLatin1Char('\t')));
    }
    return rows;
}

} // namespace

QString FlatpakAdapter::displayName() const { return QStringLiteral("Flatpak"); }
QString FlatpakAdapter::cliProgram() const { return kFlatpak; }
QStringList FlatpakAdapter::cliSearchPaths() const { return {QStringLiteral("/usr/bin")}; }
QStringList FlatpakAdapter::cliVersionArguments() const { return {QStringLiteral("--version")}; }

CliCommand FlatpakAdapter::searchCommand(const QString &query) const
{
    return {kFlatpak, {QStringLiteral("search"), query,
                       QStringLiteral("--columns=application,name,version,description,remotes")}};
}

CliCommand FlatpakAdapter::infoCommand(const QString &packageId, const QString &) const
{
    return {kFlatpak, {QStringLiteral("info"), packageId}};
}

CliCommand FlatpakAdapter::listInstalledCommand() const
{
    return {kFlatpak, {QStringLiteral("list"), QStringLiteral("--app"),
                       QStringLiteral("--columns=application,name,version,origin,description")}};
}

CliCommand FlatpakAdapter::listOutdatedCommand() const
{
    return {kFlatpak, {QStringLiteral("remote-ls"), QStringLiteral("--updates"),
                       QStringLiteral("--columns=application,name,version,origin")}};
}

CliCommand FlatpakAdapter::installCommand(const QString &packageId, const QString &) const
{
    return {kFlatpak, {QStringLiteral("install"), QStringLiteral("-y"),
                       QStringLiteral("--noninteractive"), packageId}};
}

CliCommand FlatpakAdapter::uninstallCommand(const QString &packageId, const QString &) const
{
    return {kFlatpak, {QStringLiteral("uninstall"), QStringLiteral("-y"), packageId}};
}

CliCommand FlatpakAdapter::upgradeCommand(const QString &packageId, const QString &) const
{
    return {kFlatpak, {QStringLiteral("update"), QStringLiteral("-y"), packageId}};
}

CliCommand FlatpakAdapter::upgradeAllCommand() const
{
    return {kFlatpak, {QStringLiteral("update"), QStringLiteral("-y")}};
}

CliCommand FlatpakAdapter::pinCommand(const QString &packageId, const QString &) const
{
    return {kFlatpak, {QStringLiteral("mask"), packageId}};
}

CliCommand FlatpakAdapter::unpinCommand(const QString &packageId, const QString &) const
{
    return {kFlatpak, {QStringLiteral("mask"), QStringLiteral("--remove"), packageId}};
}

QList<Package> FlatpakAdapter::parseSearch(const QString &stdOut) const
{
    // application \t name \t version \t description \t remotes
    QList<Package> result;
    const auto rows = tabRows(stdOut);
    for (const QStringList &row : rows) {
        Package p;
        p.id = row.value(0);
        p.name = row.value(1).isEmpty() ? row.value(0) : row.value(1);
        p.version = row.value(2);
        p.description = row.value(3);
        p.source = row.value(4);
        p.kind = QStringLiteral("app");
        result.append(p);
    }
    return result;
}

QList<Package> FlatpakAdapter::parseInfo(const QString &stdOut) const
{
    // Header: "Name - Description" then "Key: Value" lines.
    Package p;
    const QStringList lines = stdOut.split(QLatin1Char('\n'));
    for (const QString &raw : lines) {
        const QString line = raw.trimmed();
        if (p.name.isEmpty() && line.contains(QStringLiteral(" - "))) {
            p.name = line.section(QStringLiteral(" - "), 0, 0).trimmed();
            p.description = line.section(QStringLiteral(" - "), 1).trimmed();
            continue;
        }
        const qsizetype colon = line.indexOf(QLatin1Char(':'));
        if (colon <= 0)
            continue;
        const QString key = line.left(colon).trimmed();
        const QString value = line.mid(colon + 1).trimmed();
        if (key == QStringLiteral("ID")) p.id = value;
        else if (key == QStringLiteral("Version")) { p.installedVersion = value; p.version = value; }
        else if (key == QStringLiteral("Origin")) p.source = value;
    }
    if (p.id.isEmpty())
        return {};
    p.kind = QStringLiteral("app");
    return {p};
}

QList<Package> FlatpakAdapter::parseInstalled(const QString &stdOut) const
{
    // application \t name \t version \t origin \t description
    QList<Package> result;
    const auto rows = tabRows(stdOut);
    for (const QStringList &row : rows) {
        Package p;
        p.id = row.value(0);
        p.name = row.value(1).isEmpty() ? row.value(0) : row.value(1);
        p.installedVersion = row.value(2).isEmpty() ? QStringLiteral("?") : row.value(2);
        p.version = p.installedVersion;
        p.source = row.value(3);
        p.description = row.value(4);
        p.kind = QStringLiteral("app");
        result.append(p);
    }
    return result;
}

QList<Package> FlatpakAdapter::parseOutdated(const QString &stdOut) const
{
    // application \t name \t version \t origin
    QList<Package> result;
    const auto rows = tabRows(stdOut);
    for (const QStringList &row : rows) {
        Package p;
        p.id = row.value(0);
        p.name = row.value(1).isEmpty() ? row.value(0) : row.value(1);
        p.version = row.value(2);
        p.installedVersion = QStringLiteral("?"); // remote-ls prints the new version only
        p.source = row.value(3);
        p.kind = QStringLiteral("app");
        result.append(p);
    }
    return result;
}

QList<CliAction> FlatpakAdapter::actionCatalog() const
{
    return {
        {QStringLiteral("remove-unused"), tr("Remove unused runtimes"),
         tr("Uninstall runtimes and extensions nothing depends on anymore"),
         {kFlatpak, {QStringLiteral("uninstall"), QStringLiteral("--unused"),
                     QStringLiteral("-y")}}, false, true, true},
        {QStringLiteral("remotes"), tr("List remotes"),
         tr("Show the configured flatpak repositories"),
         {kFlatpak, {QStringLiteral("remotes")}}, false, false, false},
        {QStringLiteral("add-flathub"), tr("Add Flathub"),
         tr("Add the Flathub repository if it is not configured yet"),
         {kFlatpak, {QStringLiteral("remote-add"), QStringLiteral("--if-not-exists"),
                     QStringLiteral("flathub"),
                     QStringLiteral("https://flathub.org/repo/flathub.flatpakrepo")}},
         false, false, true},
        {QStringLiteral("history"), tr("Show history"),
         tr("Log of installs, updates and removals"),
         {kFlatpak, {QStringLiteral("history")}}, false, false, false},
        {QStringLiteral("processes"), tr("Running apps"),
         tr("Show currently running flatpak applications"),
         {kFlatpak, {QStringLiteral("ps")}}, false, false, false},
        {QStringLiteral("permissions"), tr("Show sandbox permissions"),
         tr("Print the static permissions of an application"),
         {kFlatpak, {QStringLiteral("info"), QStringLiteral("--show-permissions")}},
         true, false, false},
        {QStringLiteral("repair"), tr("Repair installation"),
         tr("Check and fix inconsistencies in the local flatpak installation"),
         {kFlatpak, {QStringLiteral("repair")}}, false, true, false},
    };
}
