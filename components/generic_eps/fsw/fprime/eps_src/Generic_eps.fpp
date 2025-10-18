module Components {
    
    @ generic_eps
    active component Generic_eps {

        @ Switch State
        enum State {
          OFF @< OFF
          ON @< ON
        }

        @ Switch Flags
        enum Flag {
          HEALTHY @< HEALTHY Operation
          UNHEALTHY_PLACEHOLDER @< Replace with Other Flags But UNHEALTHY
        }

        @ SwitchNum
        enum SW_NUM {
          SW_0 @< Switch 0
          SW_1 @< Switch 1
          SW_2 @< Switch 2
          SW_3 @< Switch 3
          SW_4 @< Switch 4
          SW_5 @< Switch 5
          SW_6 @< Switch 6
          SW_7 @< Switch 7
        }

         @ NOOP Command
        async command NOOP(
        )

         @ Reset Counters Command
        async command RESET_COUNTERS(
        )

         @ Change Switch State
        async command SWITCH(
          switch_num: SW_NUM @< Switch 0-7 to command
          switch_state: State @< ON/OFF for Switch
        )

         @ Command to Request Housekeeping
        async command REQUEST_HOUSEKEEPING(
        )

         @ Greeting event with maximum greeting length of 30 characters
        event TELEM(
            log_info: string size 40 @< 
        ) severity activity high format "Generic_eps: {}"

         @ Battery Voltage Parameter
        telemetry BatteryVoltage: F32

         @ Battery Temperature Parameter
        telemetry BatteryTemperature: F32

         @ Bus 3p3 Voltage Parameter
        telemetry Bus3p3Voltage: F32

         @ Bus 5p0 Voltage Parameter
        telemetry Bus5p0Voltage: F32

         @ Bus 12 Voltage Parameter
        telemetry Bus12Voltage: F32

         @ EPS Temperature Parameter
        telemetry EPSTemperature: F32

         @ Solar Array Voltage Parameter
        telemetry SolarArrayVoltage: F32

         @ Solar Array Temperature Parameter
        telemetry SolarArrayTemperature: F32

         @ Switch 0 Voltage Parameter
        telemetry SW0Voltage: F32

          @ Switch 0 Current Parameter
        telemetry SW0Current: F32

         @ Switch 1 Voltage Parameter
        telemetry SW1Voltage: F32

          @ Switch 1 Current Parameter
        telemetry SW1Current: F32

         @ Switch 2 Voltage Parameter
        telemetry SW2Voltage: F32

          @ Switch 2 Current Parameter
        telemetry SW2Current: F32

         @ Switch 3 Voltage Parameter
        telemetry SW3Voltage: F32

          @ Switch 3 Current Parameter
        telemetry SW3Current: F32

         @ Switch 4 Voltage Parameter
        telemetry SW4Voltage: F32

          @ Switch 4 Current Parameter
        telemetry SW4Current: F32

         @ Switch 5 Voltage Parameter
        telemetry SW5Voltage: F32

          @ Switch 5 Current Parameter
        telemetry SW5Current: F32

         @ Switch 6 Voltage Parameter
        telemetry SW6Voltage: F32

          @ Switch 6 Current Parameter
        telemetry SW6Current: F32

         @ Switch 7 Voltage Parameter
        telemetry SW7Voltage: F32

          @ Switch 7 Current Parameter
        telemetry SW7Current: F32

         @ Device Count
        telemetry DeviceCount: U32

         @ Device Error Count
        telemetry DeviceErrorCount: U32

         @ Command Count
        telemetry CommandCount: U32

         @ Command Error Count
        telemetry CommandErrorCount: U32
        
         @ Battery Raw Voltage
        telemetry RawBatteryVoltage: U16

         @ Battery Raw Temperature
        telemetry RawBatteryTemperature: U16

         @ Bus 3.3V Raw
        telemetry RawBus3p3V: U16

         @ Bus 5.0V Raw
        telemetry RawBus5p0V: U16

         @ Bus 12V Raw
        telemetry RawBus12V: U16

         @ EPS Temperature Raw
        telemetry RawEPSTemperature: U16

         @ SA Voltage Raw
        telemetry RawSAVoltage: U16

         @ SA Temperature Raw
        telemetry RawSATemperature: U16

         @ Switch 0 Raw Voltage
        telemetry RawSW0Voltage: U16

         @ Switch 0 Raw Current
        telemetry RawSW0Current: U16

         @ Switch 0 State Parameter
        telemetry SW0State: State

         @ Switch 0 Flag Parameter
        telemetry SW0Flag: Flag

         @ Switch 1 Raw Voltage
        telemetry RawSW1Voltage: U16

         @ Switch 1 Raw Current
        telemetry RawSW1Current: U16

         @ Switch 1 State Parameter
        telemetry SW1State: State

         @ Switch 1 Flag Parameter
        telemetry SW1Flag: Flag

         @ Switch 2 Raw Voltage
        telemetry RawSW2Voltage: U16

         @ Switch 2 Raw Current
        telemetry RawSW2Current: U16

         @ Switch 2 State Parameter
        telemetry SW2State: State

         @ Switch 2 Flag Parameter
        telemetry SW2Flag: Flag

         @ Switch 3 Raw Voltage
        telemetry RawSW3Voltage: U16

         @ Switch 3 Raw Current
        telemetry RawSW3Current: U16

         @ Switch 3 State Parameter
        telemetry SW3State: State

         @ Switch 3 Flag Parameter
        telemetry SW3Flag: Flag

         @ Switch 4 Raw Voltage
        telemetry RawSW4Voltage: U16

         @ Switch 4 Raw Current
        telemetry RawSW4Current: U16

         @ Switch 4 State Parameter
        telemetry SW4State: State

         @ Switch 4 Flag Parameter
        telemetry SW4Flag: Flag

         @ Switch 5 Raw Voltage
        telemetry RawSW5Voltage: U16

         @ Switch 5 Raw Current
        telemetry RawSW5Current: U16

         @ Switch 5 State Parameter
        telemetry SW5State: State

         @ Switch 5 Flag Parameter
        telemetry SW5Flag: Flag

         @ Switch 6 Raw Voltage
        telemetry RawSW6Voltage: U16

         @ Switch 6 Raw Current
        telemetry RawSW6Current: U16

         @ Switch 6 State Parameter
        telemetry SW6State: State

         @ Switch 6 Flag Parameter
        telemetry SW6Flag: Flag

         @ Switch 7 Raw Voltage
        telemetry RawSW7Voltage: U16

         @ Switch 7 Raw Current
        telemetry RawSW7Current: U16

         @ Switch 7 State Parameter
        telemetry SW7State: State

         @ Switch 7 Flag Parameter
        telemetry SW7Flag: Flag


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