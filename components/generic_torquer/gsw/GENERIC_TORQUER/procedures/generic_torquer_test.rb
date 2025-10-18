require 'cosmos'
require 'cosmos/script'
require 'generic_torquer_lib.rb'

class GENERIC_TORQUER_Functional_Test < Cosmos::Test
  def setup
    safe_generic_torquer()
  end

  def test_application
      start("tests/generic_torquer_app_test.rb")
  end

  def test_device
    start("tests/generic_torquer_device_test.rb")
  end

  def teardown
    safe_generic_torquer()
  end
end

class GENERIC_TORQUER_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_generic_torquer()
  end

  def test_AST
      start("tests/generic_torquer_ast_test.rb")
  end

  def teardown
    safe_generic_torquer()
  end
end

class Generic_torquer_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('GENERIC_TORQUER_Functional_Test')
      add_test('GENERIC_TORQUER_Automated_Scenario_Test')
  end

  def setup
    safe_generic_torquer()
  end
  
  def teardown
    safe_generic_torquer()
  end
end
