require 'cosmos'
require 'cosmos/script'
require 'mission_lib.rb'

class MGR_LPT < Cosmos::Test
  def setup

  end

  def test_lpt
      start("tests/mgr_lpt_test.rb")
  end

  def teardown

  end
end

class MGR_CPT < Cosmos::Test
  def setup
      
  end

  def test_cpt
      start("tests/mgr_cpt_test.rb")
  end

  def teardown

  end
end

class Mgr_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('MGR_CPT')
      add_test('MGR_LPT')
  end

  def setup
  end
  
  def teardown
  end
end