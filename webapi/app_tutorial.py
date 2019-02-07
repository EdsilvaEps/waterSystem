#!flask/bin/python
from flask import Flask, jsonify, abort, make_response
from flask_restful import reqparse, Api, Resource, fields, marshal
from flask_httpauth import HTTPBasicAuth

app = Flask(__name__)
api = Api(app)
auth = HTTPBasicAuth()

@auth.get_password
def get_password(username):
    # this would usually check the database
    if username == 'eddie':
        return 'python'
    return None

@auth.error_handler
def unauthorized():
    # response customized to return json instead of HTML
    return make_response(jsonify({'error':'unauthorized access'}), 401)


tasks = [
    {
        'id': 1,
        'title': u'Buy groceries',
        'description': u'Milk, Cheese, Pizza, Fruit, Tylenol',
        'done': False
    },
    {
        'id': 2,
        'title': u'Learn Python',
        'description': u'Need to find a good Python tutorial on the web',
        'done': False
    }
]

'''
the task_fields structure serves as template for the
'marshal' function (field conversion). The fields.Url
type is a special type that generates a URL.
'''
task_fields = {
    'title' : fields.String,
    'description': fields.String,
    'done': fields.Boolean,
    'uri': fields.Url('task')
}

'''
the Resource base class can define the routing
for one or more HTTP methods for a given URL.
'''
class TaskListAPI(Resource):
    # decorator to protect the functions
    # against unauthorized access
    decorators = [auth.login_required]

    '''
    in the TaskListAPI resource, the post method is the only one
    that receives arguments. The 'title' argument is required here
    so a response is sent when the field is missing. The 'description'
    field is optional, and a default "" value is added when it is missing.
    '''
    def __init__(self):
        # defining arguments and how to validade them
        self.reqparse = reqparse.RequestParser()
        self.reqparse.add_argument('title', type=str, required= True,
            help = 'No task title provided', location='json')
            # the 'location' argument refers to request.json, where 'request'
            # is the body of the post request
        self.reqparse.add_argument('description',type=str, default="",location='json')
        super(TaskListAPI,self).__init__()

    def get(self):
        return {'tasks':[marshal(task, task_fields) for task in tasks]}

    def post(self):
        args = self.reqparse.parse_args()
        task = {
            'id': tasks[-1]['id'] + 1,
            'title': args['title'],
            'description': args['description'],
            'done': False
        }
        tasks.append(task)
        return {'task': marshal(task, task_fields)}, 201

# TaskAPI receives the 'id' argument
# to handle singular tasks
class TaskAPI(Resource):
    decorators = [auth.login_required]
    '''
    on the TaskAPI only the put method will need to parse argument
    and for this method all the arguments are optional, including the
    done field.
    '''
    def __init__(self):
        self.reqparse = reqparse.RequestParser()
        self.reqparse.add_argument('title', type = str, location = 'json')
        self.reqparse.add_argument('description', type = str, location = 'json')
        self.reqparse.add_argument('done', type = bool, location = 'json')
        super(TaskAPI, self).__init__()

    def get(self, id):
        task = [task for task in tasks if task['id'] == id]
        if len(task) == 0:
            abort(404)
        return {'task': marshal(task[0], task_fields)}

    def put(self, id):
        # looking for task with matching id in tasks dictionary
        task = [task for task in tasks if task['id'] == id]
        if len(task) == 0:
            abort(404)
        task = task[0]
        args = self.reqparse.parse_args()
        for k, v in args.items():
            if v is not None:
                task[k] = v
        '''
        instead of using jsonify to generate responses
        flask_restful automatically handles conversion to JSON
        and supports passing a custom status code when necessary:
        '''
        # old way: return jsonify( { 'task': make_public_task(task) } )
        return  {'task':marshal(task, task_fields) }, 201

    def delete(self, id):
        task = [task for task in tasks if task['id'] == id]
        if len(task) == 0:
            abort(404)
        tasks.remove(task[0])
        return {'result': True}

'''
the add_resource function registers the routes with the
framework using the given endpoint.
'''
api.add_resource(TaskListAPI, '/todo/api/v1.0/tasks', endpoint='tasks')
api.add_resource(TaskAPI, '/todo/api/tasks/<int:id>', endpoint='task')

if __name__ == '__main__':
    app.run(debug=True)
