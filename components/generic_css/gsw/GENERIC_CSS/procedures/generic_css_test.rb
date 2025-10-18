require 'cosmos'
require 'cosmos/script'
require 'generic_css_lib.rb'

class GENERIC_CSS_Functional_Test < Cosmos::Test
  def setup
    safe_generic_css()
  end

  def test_application
      start("tests/generic_css_app_test.rb")
  end

  def test_device
    start("tests/generic_css_device_test.rb")
  end

  def teardown
    safe_generic_css()
  end
end

class GENERIC_CSS_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_generic_css()
  end

  def test_cpt
      start("tests/generic_css_ast_test.rb")
  end

  def teardown
    safe_generic_css()
  end
end

class Generic_css_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('GENERIC_CSS_Functional_Test')
      add_test('GENERIC_CSS_Automated_Scenario_Test')
  end

  def setup
    safe_generic_css()
  end
  
  def teardown
    safe_generic_css()
  end
end
