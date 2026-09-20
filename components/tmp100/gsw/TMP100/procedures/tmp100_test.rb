require 'cosmos'
require 'cosmos/script'
require 'tmp100_lib.rb'

class TMP100_Functional_Test < Cosmos::Test
  def setup
    safe_tmp100()
  end

  def test_application
      start("tests/tmp100_app_test.rb")
  end

  def test_device
    start("tests/tmp100_device_test.rb")
  end

  def teardown
    safe_tmp100()
  end
end

class TMP100_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_tmp100()
  end

  def test_AST
      start("tests/tmp100_ast_test.rb")
  end

  def teardown
    safe_tmp100()
  end
end

class Tmp100_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('TMP100_Functional_Test')
      add_test('TMP100_Automated_Scenario_Test')
  end

  def setup
    safe_tmp100()
  end
  
  def teardown
    safe_tmp100()
  end
end
