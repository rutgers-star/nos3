require 'cosmos'
require 'cosmos/script'
require 'arducam_lib.rb'

class ARDUCAM_Functional_Test < Cosmos::Test
  def setup
    safe_arducam()
  end

  def test_application
      start("tests/arducam_app_test.rb")
  end

  def test_device
    start("tests/arducam_device_test.rb")
  end

  def teardown
    safe_arducam()
  end
end

class ARDUCAM_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_arducam()
  end

  def test_AST
      start("tests/arducam_ast_test.rb")
  end

  def teardown
    safe_arducam()
  end
end

class Arducam_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('ARDUCAM_Functional_Test')
      add_test('ARDUCAM_Automated_Scenario_Test')
  end

  def setup
    safe_arducam()
  end
  
  def teardown
    safe_arducam()
  end
end
