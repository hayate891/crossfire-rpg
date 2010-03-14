#include "CRESettings.h"
#include "CRESettingsDialog.h"

#include <QFileInfo>
#include <QDir>

CRESettings::CRESettings()
{
}

bool CRESettings::ensureOptions()
{
    bool need = false;
    if (mapCacheDirectory().isEmpty())
        need = true;
    else
    {
        QFileInfo info(mapCacheDirectory());
        if (!info.exists())
        {
            QDir dir(mapCacheDirectory());
            dir.mkpath(".");
            info.refresh();
        }

        if (!info.exists())
            need = true;
    }

    if (need)
    {
        CRESettingsDialog dlg(this);
        if (dlg.exec() == QDialog::Rejected)
            return false;

        setValue("mapCacheDirectory", dlg.mapCache());
        return ensureOptions();
    }

    return true;
}

QString CRESettings::mapCacheDirectory() const
{
    return value("mapCacheDirectory").toString();
}
