from flask import Flask, render_template, request
import json
from GPIOController import GPIOController

app = Flask(__name__)
gpio_handler = GPIOController()


@app.route('/')
def homepage():
    automaton_state = gpio_handler.read_input()
    return render_template('homepage.html', automaton_state=automaton_state)


@app.route('/api', methods=['POST'])
def control_feed_automaton():
    if request.method == "POST":
        data = request.get_json()
    gpio_handler.set_output(data['value'])
    return json.dumps({'success': True}), 200, {'ContentType': 'application/json'}


if __name__ == '__main__':
    app.debug = True
    try:
        app.run(host = '0.0.0.0',port=5005)
    except KeyboardInterrupt:
        print('exit') 
    finally:  
        gpio_handler.cleanup # this ensures a clean exit  
