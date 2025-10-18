module Components {
    @ Satellite Thruster
    active component Generic_thruster {

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port
        #@ TODO
        #async command TODO opcode 0

        enum ActiveState {
            DISABLED @< DISABLED
            ENABLED @< ENABLED
        }

        enum thrusterNums {
            thruster0 @< thruster 0
            thruster1 @< thruster 1
            thruster2 @< thruster 2
            thruster3 @< thruster 3
        }

        @ NOOP Command
        async command NOOP()

        @ Enable Cmd
        async command ENABLE()

        @ Disable Cmd
        async command DISABLE()

        @ Reset Counters Cmd
        async command RESET_COUNTERS()

        @ Command to Request Housekeeping
        async command REQUEST_HOUSEKEEPING()

        @ Command to issue greeting with maximum length of 20 characters
        async command SET_PERCENTAGE(
            percent: U8 @< Percent speed of rotation, 0 to 100
            thruster_number: thrusterNums @< Direction of rotation
        )

        @ Greeting event with maximum greeting length of 60 characters
        event TELEM(
            log_info: string size 60 @< 
        ) severity activity high format "Generic_thruster: {}"

        @ Percentage thruster is being set to
        telemetry Percentage_0: U8

        @ Percentage thruster is being set to
        telemetry Percentage_1: U8

        @ Percentage thruster is being set to
        telemetry Percentage_2: U8

        @ Percentage thruster is being set to
        telemetry Percentage_3: U8

        @ Command Count
        telemetry CommandCount: U32

        @ Command Error Count
        telemetry CommandErrorCount: U32

        @ Device Count
        telemetry DeviceCount: U32

        @ Device Error Count
        telemetry DeviceErrorCount: U32

        @ Device Enabled
        telemetry DeviceEnabled: ActiveState

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