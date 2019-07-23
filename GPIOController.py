import RPi.GPIO as GPIO


class GPIOController:
	def __init__(self):
		# use P1 header pin numbering convention
		GPIO.setmode(GPIO.BOARD)
		# Set up the GPIO channels - one input and one output
		GPIO.setup(31, GPIO.IN)
		GPIO.setup(35, GPIO.OUT)

	def read_input(self, pin=31):
		read_value = GPIO.input(pin)
		return read_value

	def set_output(self, value, pin=35):
		print(value)
		# GPIO.HIGH / GPIO.LOW
		GPIO.output(pin, value)

	def cleanup(self):
		GPIO.cleanup()