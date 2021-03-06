// -*- coding: utf-8 -*-
// Copyright (C) 2012 Gustavo Puche, Rosen Diankov, OpenGrasp Team
//
// OpenRAVE Qt/OpenSceneGraph Viewer is licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "qtosg.h"
#include "qtosgviewer.h"
#include <openrave/plugin.h>

using namespace OpenRAVE;

#if defined(HAVE_X11_XLIB_H) && defined(Q_WS_X11)
#include "X11/Xlib.h"
#endif

// for some reason windows complains when the prototypes are different
InterfaceBasePtr CreateInterfaceValidated(InterfaceType type, const std::string& interfacename, std::istream& sinput, EnvironmentBasePtr penv)
{
    //	Debug.
    RAVELOG_VERBOSE("Initiating QTOSGRave plugin...!!!!.\n");
    
    static int s_SoQtArgc = 0; // has to be static!!

    switch(type) {
    case PT_Viewer:
#if defined(HAVE_X11_XLIB_H) && defined(Q_WS_X11)
        // always check viewers since DISPLAY could change
        if ( XOpenDisplay( NULL ) == NULL ) {
            RAVELOG_WARN("no display detected, so cannot load viewer");
            return InterfaceBasePtr();
        }
#endif
        if( interfacename == "qtosg" ) {
            QApplication *app= new QApplication(s_SoQtArgc,NULL);
            return InterfaceBasePtr(new qtosgrave::QtOSGViewer(penv,app));
        }
        break;
    default:
        break;
    }

    return InterfaceBasePtr();
}

void GetPluginAttributesValidated(PLUGININFO& info)
{
    info.interfacenames[PT_Viewer].push_back("qtosg");
}

OPENRAVE_PLUGIN_API void DestroyPlugin()
{
}
