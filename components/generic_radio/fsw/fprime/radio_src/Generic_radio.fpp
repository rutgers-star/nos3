module Components {
    @ generic_radio from nos3
    active component Generic_radio {

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port

        @ Command to Request Housekeeping
        async command REQUEST_HOUSEKEEPING()

        @ Command to Request Noop
        async command NOOP()

        @ Config Command
        async command CONFIG(
            config_value: U32
        )

        @ Reset Counters Command
        async command RESET_COUNTERS()

        @ event with maximum length of 40 characters
        event TELEM(
            log_info: string size 40 @< 
        ) severity activity high format "Generic_radio: {}"

        @ Command Count
        telemetry CommandCount: U32

        @ Command Error Count
        telemetry CommandErrorCount: U32

        @ Device Count
        telemetry DeviceCount: U32

        @ Device Error Count
        telemetry DeviceErrorCount: U32

        @ A count of the number of greetings issued
        telemetry ReportedComponentCount: U32

         @ A count of the number of greetings issued
        telemetry DeviceConfig: U32

         @ A count of the number of greetings issued
        telemetry ProxySignal: U32

        ##############################################################################
        #### Uncomment the following examples to start customizing your component ####
        ##############################################################################

        # @ Example async command
        # async command COMMAND_NAME(param_name: U32)

        # @ Example telemetry counter
        # telemetry ExampleCounter: U64

        # @ Example event
        # event ExampleStateEvent(example_state: Fw.On) severity activity high id 0 format "State set to {}"

        # @ Example port: receiving calls from the rate group
        # sync input port run: Svc.Sched

        # @ Example parameter
        # param PARAMETER_NAME: U32

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