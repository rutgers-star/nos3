require 'cosmos'
require 'cosmos/script'
require 'sample_lib.rb'

class SAMPLE_Functional_Test < Cosmos::Test
  def setup
    safe_sample()
  end

  def test_application
      start("tests/sample_app_test.rb")
  end

  def test_device
    start("tests/sample_device_test.rb")
  end

  def teardown
    safe_sample()
  end
end

class SAMPLE_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_sample()
  end

  def test_AST
      start("tests/sample_ast_test.rb")
  end

  def teardown
    safe_sample()
  end
end

class Sample_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('SAMPLE_Functional_Test')
      add_test('SAMPLE_Automated_Scenario_Test')
  end

  def setup
    safe_sample()
  end
  
  def teardown
    safe_sample()
  end
end
