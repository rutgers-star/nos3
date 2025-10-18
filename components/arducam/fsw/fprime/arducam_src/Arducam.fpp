module Components {
    @ Arducam component from NOS3
    active component Arducam {

        @ Image Size
        enum ImageSize {
            Small @< Small (0)
            Medium @< Medium (1)
            Large @< Large (2)
        }
        
        @ Command to connect I2C
        async command I2C(
        )

        @ Command to connect SPI
        async command SPI(
        )

        @ Command to request an image 
        async command IMAGE(
            image_size: ImageSize @< 0 (small), 1 (medium), or 2 (large)
        )

        @ Command to send NOOP
        async command NOOP(
        )

        @ Command to Reset Counters
        async command RESET_COUNTERS(
        )

        @ Command to Report HouseKeeping
        async command REPORT_HOUSEKEEPING(
        )

        async command HARDWARE_CHECKOUT(
        )

        @ Telemetry event 
        event TELEM(
            log_info: string size 60 
        ) severity activity high format "Arducam: {}"

        @ Command Count
        telemetry CommandCount: U32

        @ Command Error Count
        telemetry CommandErrorCount: U32

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
