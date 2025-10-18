require 'cosmos'
require 'cosmos/script'
require 'generic_st_lib.rb'

class GENERIC_STAR_TRACKER_Functional_Test < Cosmos::Test
  def setup
    safe_generic_star_tracker()
  end

  def test_application
      start("tests/generic_st_app_test.rb")
  end

  def test_device
      start("tests/generic_st_device_test.rb")
  end

  def teardown
    safe_generic_star_tracker()
  end
end

class GENERIC_STAR_TRACKER_Automated_Scenario_Test < Cosmos::Test
  def setup 
      safe_generic_star_tracker()
  end

  def test_AST
      start("tests/generic_st_ast_test.rb")
  end

  def teardown
      safe_generic_star_tracker()
  end
end

class Generic_st_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('GENERIC_STAR_TRACKER_Functional_Test')
      add_test('GENERIC_STAR_TRACKER_Automated_Scenario_Test')
  end

  def setup
    safe_generic_star_tracker()
  end
  
  def teardown
    safe_generic_star_tracker()
  end
end
