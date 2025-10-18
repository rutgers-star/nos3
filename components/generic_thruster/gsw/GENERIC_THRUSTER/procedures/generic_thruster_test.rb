require 'cosmos'
require 'cosmos/script'
require 'generic_thruster_lib.rb'

class GENERIC_THRUSTER_Functional_Test < Cosmos::Test
  def setup
    safe_generic_thruster()
  end

  def test_application
      start("tests/generic_thruster_app_test.rb")
  end

  def test_device
    start("tests/generic_thruster_device_test.rb")
  end

  def teardown
    safe_generic_thruster()
  end
end

class GENERIC_THRUSTER_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_generic_thruster()
  end

  def test_AST
      start("tests/generic_thruster_ast_test.rb")
  end

  def teardown
    safe_generic_thruster()
  end
end

class Generic_thruster_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('GENERIC_THRUSTER_Functional_Test')
      #add_test('GENERIC_THRUSTER_Automated_Scenario_Test')
  end

  def setup
    safe_generic_thruster()
  end
  
  def teardown
    safe_generic_thruster()
  end
end
