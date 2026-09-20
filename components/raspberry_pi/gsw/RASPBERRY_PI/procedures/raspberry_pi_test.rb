require 'cosmos'
require 'cosmos/script'
require 'raspberry_pi_lib.rb'

class RASPBERRY_PI_Functional_Test < Cosmos::Test
  def setup
    safe_raspberry_pi()
  end

  def test_application
      start("tests/raspberry_pi_app_test.rb")
  end

  def test_device
    start("tests/raspberry_pi_device_test.rb")
  end

  def teardown
    safe_raspberry_pi()
  end
end

class RASPBERRY_PI_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_raspberry_pi()
  end

  def test_AST
      start("tests/raspberry_pi_ast_test.rb")
  end

  def teardown
    safe_raspberry_pi()
  end
end

class Raspberry_pi_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('RASPBERRY_PI_Functional_Test')
      add_test('RASPBERRY_PI_Automated_Scenario_Test')
  end

  def setup
    safe_raspberry_pi()
  end
  
  def teardown
    safe_raspberry_pi()
  end
end
