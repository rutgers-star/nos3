module Components {
    @ Satellite Torquer
    active component Generic_torquer {

        @ Component Enable State
        enum ActiveState {
            DISABLED @< DISABLED
            ENABLED @< ENABLED
        }

        @ Torquer Number
        enum tq_num {
            Torquer_0 @< Torquer 0
            Torquer_1 @< Torquer 1
            Torquer_2 @< Torquer 2
        }

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port

         @ NOOP Cmd
        async command NOOP(
        )

         @ RESET COUNTERS cmd
        async command RESET_COUNTERS(
        )

        async command ENABLE(
        )

        async command DISABLE(
        )

        async command REQUEST_HOUSEKEEPING(
        )

        async command ALL_CONFIG(
            Percent_0: U8 @< Percent speed of rotation: (0-100)
            Direction_0: U8 @< Direction of rotation (0 or 1)
            Percent_1: U8 @< Percent speed of rotation: (0-100)
            Direction_1: U8 @< Direction of rotation (0 or 1)
            Percent_2: U8 @< Percent speed of rotation: (0-100)
            Direction_2: U8 @< Direction of rotation (0 or 1)
        )

        @ Command to issue greeting with maximum length of 20 characters
        async command GENERIC_TORQUER_CONFIG(
            torquerNum: tq_num @< Torquer Number
            Percent: U8 @< Percent speed of rotation: (0 - 100)
            Direction: U8 @< Direction of rotation (0 or 1)
        )

        @ Greeting event with maximum greeting length of 30 characters
        event TELEM(
            log_info: string size 40 @< 
        ) severity activity high format "Generic_torquer: {}"

         @ Command Count
        telemetry CommandCount: U32

         @ Command Error Count
        telemetry CommandErrorCount: U32

         @ Device Count
        telemetry DeviceCount: U32

         @ Device Error Count
        telemetry DeviceErrorCount: U32

         @ Device Enable State
        telemetry DeviceEnabled: ActiveState

        @ A count of the number of greetings issued
        telemetry Percent_0: U8
        
        @ A count of the number of greetings issued
        telemetry Direction_0: U8
        
        @ A count of the number of greetings issued
        telemetry Percent_1: U8
        
        @ A count of the number of greetings issued
        telemetry Direction_1: U8
        
        @ A count of the number of greetings issued
        telemetry Percent_2: U8
        
        @ A count of the number of greetings issued
        telemetry Direction_2: U8

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