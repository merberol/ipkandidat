# Liu Haptic interface for X-Plane

## Main files
### Source
    XplaneHapticInterface.cpp
    haptic_library.dll
    python39.dll
    python39_d.dll
#### src/
    ConfigLoader.hpp
    EventHandler.hpp
    pyFuction.hpp
    types.h 
#### libs/
    python39_d.lib
    python39.lib
##### bHapticSDK/
##### xplane/
    XPLM_64.lib
    XPLM.lib
    XP_widgets_64.lib
    XP_widgets.lib
#### includes/
##### bHaptics/   
    HapticLibrary.h
    model.h
##### Python39/
    copied all .h files from original install place
##### xplane/CHeaders
    all


### program @ C:\X-Plane 11\Resources\plugins

#### LiuHaptics
    win.xpl <--- main plugin file
##### ConfigCSV.csv  
    this files contains the nessesary data for the plugin to run properly. 
    it is constructed on the form
    EventName;TactFileName;Used;PyFile;NumDataRefs;type;dataref;type;dataref;type;dataref;type;dataref;type;dataref
    ReadyEvent;ReadyEvent.tact;1;Resources.plugins.LiuHaptics.PythonFiles.doReady;0;;;;;;;;;;

    a tabulated version for clarity showing only the ready event and expludes empty slots
    EventName   ;   TactFileName    ;   Used    ;   PyFile                                              ;   NumDataRefs ;
    ReadyEvent  ;   ReadyEvent.tact ;   1       ;   Resources.plugins.LiuHaptics.PythonFiles.doReady    ;   0           ;

    Eventname 
        The name under wich the event shall be registered
        is used by both human user and internally in the system.
    TactFileName
        the full name of the tact files assosiated  with the event 
        only the file name not the entire file path
    Used
        1 if you want this event to fire 0 if you for some reason want to disable it but not remove it.
        IMPORTANT
        any event named 'ReadyEvent' will only run once and there shuld only be one of them.
    PyFile
        the full include path of the python module the hold the main function to determine if an event shuld fire.

        for more information see the detailed description of pyfiles below.
    NumDataRefs
        the Number of arguments needed for the python function call
    Number amount of type;dataref pairs
        a Type is either a 'int' or a 'float'
        bools are ints confined to 0 or 1 as values
        datarefs are url like strings that target specific datapoints in the XPlane API
##### PythonFiles
    Put event logic files here

    the python file it self needs a main() function that returns a integer with either the value 1 or 0 weather or not the event shuld run. 

##### tactFiles
    Put .tact files here



