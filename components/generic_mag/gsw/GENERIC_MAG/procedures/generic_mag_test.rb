require 'cosmos'
require 'cosmos/script'
require 'generic_mag_lib.rb'

class GENERIC_MAG_Functional_Test < Cosmos::Test
  def setup
    safe_generic_mag()
  end

  def test_application
      start("tests/generic_mag_app_test.rb")
  end

  def test_device
    start("tests/generic_mag_device_test.rb")
  end

  def teardown
    safe_generic_mag()
  end
end

class GENERIC_MAG_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_generic_mag()
  end

  def test_AST
      start("tests/generic_mag_ast_test.rb")
  end

  def teardown
    safe_generic_mag()
  end
end

class Generic_mag_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('GENERIC_MAG_Functional_Test')
      add_test('GENERIC_MAG_Automated_Scenario_Test')
  end

  def setup
    safe_generic_mag()
  end
  
  def teardown
    safe_generic_mag()
  end
end
