require 'cosmos'
require 'cosmos/script'
require 'payload_if_lib.rb'

class PAYLOAD_IF_Functional_Test < Cosmos::Test
  def setup
    safe_payload_if()
  end

  def test_application
      start("tests/payload_if_app_test.rb")
  end

  def test_device
    start("tests/payload_if_device_test.rb")
  end

  def teardown
    safe_payload_if()
  end
end

class PAYLOAD_IF_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_payload_if()
  end

  def test_AST
      start("tests/payload_if_ast_test.rb")
  end

  def teardown
    safe_payload_if()
  end
end

class Payload_if_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('PAYLOAD_IF_Functional_Test')
      add_test('PAYLOAD_IF_Automated_Scenario_Test')
  end

  def setup
    safe_payload_if()
  end
  
  def teardown
    safe_payload_if()
  end
end
