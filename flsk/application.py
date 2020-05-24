from flask import Flask
from flask import request
from flask import render_template
app = Flask(__name__)

@app.route("/")
def hello():
    with open("f.txt", "r") as f:
        req = f.readlines()[-1]
    return render_template('hello.html', name=req, tmp=[1,2,3])

@app.route('/tmp/<tmp>/hum/<hum>')
def show_user_profile(tmp,hum):
    # показать профиль данного пользователя
    return 'tmp %s hum %s' % (tmp, hum)



@app.route('/login')
def login():
    with open("f.txt", 'a') as f:
        f.write(request.args['tmp'] + "\n")
    return request.args['tmp']