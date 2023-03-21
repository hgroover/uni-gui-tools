#ifndef ARCHIVEMANAGERGLOBALS_H
#define ARCHIVEMANAGERGLOBALS_H

#define MYQSETTINGS(instanceName) QSettings instanceName("archive-manager.ini", QSettings::IniFormat)

#define MY_APP_VER "1.0.8"

static const char *cfg_cw_geometry = "cw_geometry";
static const char *cfg_cw_state = "cw_state";
static const char *cfg_logDir = "logDir";
static const char *cfg_logFilespec = "logFilespec";
static const char *cfg_gitDir = "gitDir";
static const char *cfg_extractionDir = "extractionDir";
static const char *cfg_extractionScript = "extractionScript";
static const char *cfg_baseUrl = "baseUrl";
static const char *cfg_viewSizeTrigger = "viewSizeTrigger";
static const char *cfg_viewExternalViewer = "viewExternalViewer";

static const char *cfg_mw_geometry = "mw_geometry";
static const char *cfg_mw_state = "mw_state";

static const char *cfg_dw_geometry = "dw_geometry";
static const char *cfg_dw_state = "dw_state";


#endif // ARCHIVEMANAGERGLOBALS_H
