module Components {
    @ fine sun sensor component from NOS3
    active component Generic_fss {

        ##############################################################################
        #### Uncomment the following examples to start customizing your component ####
        ##############################################################################

        @ Component Enable State
        enum ActiveState {
            DISABLED @< DISABLED
            ENABLED @< ENABLED
        }

        @ NOOP Cmd
        async command NOOP()

        @ Reset Counters Cmd
        async command RESET_COUNTERS()

        @ Enable Cmd
        async command ENABLE()

        @ Disable Cmd
        async command DISABLE()

        @ Request Housekeeping
        async command REQUEST_HOUSEKEEPING()

        @ Command to request data
        async command REQUEST_DATA(
        )

        @ Telemetry event
        event TELEM(
            log_info: string size 40
        ) severity activity high format "Generic_fss: {}"

        @ Command Count
        telemetry CommandCount: U32

        @ Command Error Count
        telemetry CommandErrorCount: U32

        @ Device Count
        telemetry DeviceCount: U32

        @ Device Error Count
        telemetry DeviceErrorCount: U32

        telemetry DeviceEnabled: ActiveState

        @ Angle alpha
        telemetry ALPHA: U32

        @ Angle beta
        telemetry BETA: U32

        @ errorcode
        telemetry ERRORCODE: U8



        # @ Example port: receiving calls from the rate group
        # sync input port run: Svc.Sched

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Port to return the value of a parameter
        param get port prmGetOut

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}
