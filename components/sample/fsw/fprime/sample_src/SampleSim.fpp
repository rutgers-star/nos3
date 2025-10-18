module Components {
    @ Component for F Prime FSW framework.
    active component SampleSim {

        @ Component Enable State
        enum ActiveState {
            DISABLED @< DISABLED
            ENABLED @< ENABLED
        }

        # # One async command/port is required for active components
        @ Command to Request Housekeeping
        async command REQUEST_HOUSEKEEPING(
        )

        @ Command to issue noop
        async command NOOP(
        )

        @ Command to initialize Sequence Sample HK
        async command SAMPLE_SEQ(
        )

        async command ENABLE(
        )

        async command DISABLE(
        )

        async command RESET_COUNTERS(
        )

        async command CONFIGURE(
            config: U32 @< Configuration Integer
        )

        #@ Command to initialize Sequence Sample HK
        #async command SAMPLE_SEQ_CANCEL(
        #)priority 10

        @ Greeting event with maximum greeting length of 40 characters
        event TELEM(
            log_info: string size 40 @< 
        ) severity activity high format "SampleSim: {}"

        #@ text event with maximum length of 30 characters
        #event TEXTTELEM severity diagnostic format "This is testing noop"

         @ A count of the number of greetings issued
        telemetry ReportedComponentCount: U32

         @ Device Config
        telemetry DeviceConfig: U32

         @ Device Status
        telemetry DeviceStatus: U32

        @ Device Count
        telemetry DeviceCount: U32

        @ Device Error Count
        telemetry DeviceErrorCount: U32

        @ A Count of Successful Commands from FSW to Sample
        telemetry CommandCount: U32

        @ A Count of Unsuccessful Commands from FSW to Sample
        telemetry CommandErrorCount: U32

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
        #text event port textEventOut
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