require 'cosmos'
require 'cosmos/script'
require 'generic_fss_lib.rb'

class GENERIC_FSS_Functional_Test < Cosmos::Test
  def setup
    safe_fss()
  end

  def test_application
      start("tests/generic_fss_app_test.rb")
  end

  def test_device
    start("tests/generic_fss_device_test.rb")
  end

  def teardown
    safe_fss()
  end
end

class GENERIC_FSS_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_fss()
  end

  def test_AST
      start("tests/generic_fss_ast_test.rb")
  end

  def teardown
    safe_fss()
  end
end

class Generic_fss_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('GENERIC_FSS_Functional_Test')
      add_test('GENERIC_FSS_Automated_Scenario_Test')
  end

  def setup
    safe_fss()
  end
  
  def teardown
    safe_fss()
  end
end
