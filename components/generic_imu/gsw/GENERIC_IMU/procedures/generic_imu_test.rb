require 'cosmos'
require 'cosmos/script'
require 'generic_imu_lib.rb'

class GENERIC_IMU_Functional_Test < Cosmos::Test
  def setup
    safe_generic_imu()
  end

  def test_application
      start("tests/generic_imu_app_test.rb")
  end

  def test_device
    start("tests/generic_imu_device_test.rb")
  end

  def teardown
    safe_generic_imu()
  end
end

class GENERIC_IMU_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_generic_imu()
  end

  def test_AST
      start("tests/generic_imu_ast_test.rb")
  end

  def teardown
    safe_generic_imu()
  end
end

class Generic_imu_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('GENERIC_IMU_Functional_Test')
      add_test('GENERIC_IMU_Automated_Scenario_Test')
  end

  def setup
    safe_generic_imu()
  end
  
  def teardown
    safe_generic_imu()
  end
end
