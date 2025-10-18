require 'cosmos'
require 'cosmos/script'
require 'generic_eps_lib.rb'

class GENERIC_EPS_Functional_Test < Cosmos::Test
  def setup
    safe_eps()
  end

  def test_application
      start("tests/generic_eps_app_test.rb")
  end

  def test_device
    start("tests/generic_eps_device_test.rb")
  end

  def teardown
    safe_eps()
  end
end

class GENERIC_EPS_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_eps()
  end

  def test_AST
      start("tests/generic_eps_ast_test.rb")
  end

  def teardown
    safe_eps()
  end
end

class Generic_eps_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('GENERIC_EPS_Functional_Test')
      add_test('GENERIC_EPS_Automated_Scenario_Test')
  end

  def setup
    safe_eps()
  end
  
  def teardown
    safe_eps()
  end
end
