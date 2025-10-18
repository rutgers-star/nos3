require 'cosmos'
require 'cosmos/script'
require 'gps_lib.rb'

class NOVATEL_OEM615_Functional_Test < Cosmos::Test
  def setup
    safe_gps()
  end

  def test_application
      start("tests/novatel_oem615_app_test.rb")
  end

  def test_device
    start("tests/novatel_oem615_device_test.rb")
  end

  def teardown
    safe_gps()
  end
end

class NOVATEL_OEM615_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_gps()
  end

  def test_AST
      start("tests/novatel_oem615_ast_test.rb")
  end

  def teardown
    safe_gps()
  end
end

class Novatel_oem615_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('NOVATEL_OEM615_Functional_Test')
      add_test('NOVATEL_OEM615_Automated_Scenario_Test')
  end

  def setup
    safe_gps()
  end
  
  def teardown
    safe_gps()
  end
end
