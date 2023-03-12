# Plugin developer guide

Plugins are bash scripts (executed via bash or dash on Linux, git bash on Windows) which use
JSON to express their interface to the GUI.

A plugin is an executable shell script which needs to handle one special argument __getdefs
and responds by returning JSON defining the GUI elements needed to operate.

GUI elements are used to define elements that get passed to the script as arguments or
environment variables.

Plugins reside in a specified directory and use the extension .am-plugin. They should have
the executable bit set (already the default when saving files in windows for git bash).

Here's a simple example plugin which might be saved as ~/plugins/hello.am-plugin

```
#!/bin/sh
# Example archive-manager plugin

[ "$1" = "__getdefs" ] && {
  cat <<__GETDEFS__
{
  "PluginSpec": 1.0,
  "Visible": true,
  "Form": {
    "Title": "Hello example",
    "Fields": [
      {
        "ID": "Name",
        "PassAs": "arg",
        "Index": 1,
        "Label": "Your name:",
        "Type": "text",
        "Persist": false,
        "Required": true
      },
      {
        "ID": "Dow",
        "PassAs": "arg",
        "Index": 2,
        "Label": "Day of week:",
        "Type": "dropdown",
        "Persist": false,
        "Required": true,
        "Elements": [
          { "Label": "Sunday", "Value": "Sun" },
          { "Label": "Mon" },
          { "Label": "Tue" },
          { "Label": "Wed" },
          { "Label": "Thu" },
          { "Label": "Fri" },
          { "Label": "Sat" }
        ]
      },
      {
        "ID": "Weather",
        "PassAs": "env",
        "EnvName": "WEATHER",
        "Label": "Weather condition:",
        "Type": "text",
        "Persist": false,
        "Required": false,
        "Default": "sunny"
    ]
  }
}
__GETDEFS__
  exit 0
}

echo "Hello $1 it's $2 and the weather is ${WEATHER}"
```
