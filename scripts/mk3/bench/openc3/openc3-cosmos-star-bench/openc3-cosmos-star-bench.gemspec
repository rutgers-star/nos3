# encoding: ascii-8bit

# Create the overall gemspec
Gem::Specification.new do |s|
  s.name = 'openc3-cosmos-star-bench'
  s.summary = 'STAR busOBC to payOBC bench link'
  s.description = <<-EOF2
    Commands and telemetry for the real busOBC (SDR MK3) and payOBC (Raspberry Pi) on the bench
  EOF2
  s.license = 'MIT'
  s.authors = ['STAR']
  s.email = ['name@domain.com']
  s.homepage = 'https://github.com/rutgers-star/nos3'
  s.platform = Gem::Platform::RUBY

  if ENV['VERSION']
    s.version = ENV['VERSION'].dup
  else
    time = Time.now.strftime("%Y%m%d%H%M%S")
    s.version = '0.0.0' + ".#{time}"
  end
  s.files = Dir.glob("{targets,lib,tools,microservices}/**/*") + %w(Rakefile README.md LICENSE.txt plugin.txt)
end
