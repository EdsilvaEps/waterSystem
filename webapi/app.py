#!flask/bin/python
from flask import Flask, jsonify, abort, make_response
from flask_restful import reqparse, Api, Resource, fields, marshal
from flask_httpauth import HTTPBasicAuth

app = Flask(__name__)
api = Api(app)
auth = HTTPBasicAuth()

'''
test object of pumping order for the watering system
If successful, we should move it to a sqlite database
'''
orders = [
    {
        'id': 1,
        'title': 'winter',
        'description': 'waters plants once a day',
        'interval': [24.00],
        'amount': 1000
    },
    {
        'id': 2,
        'title': 'summer',
        'description':'waters plants twice a day',
        'interval': [5.00, 24.00],
        'amount': 3000
    },

]

order_fields = {
    'title': fields.String,
    'description': fields.String,
    'interval': fields.List(fields.Float), # might not work
    'amount': fields.Integer
}

class OrderListAPI(Resource):
    # add this once login is implemented
    # decorators = [auth.login_required]

    def __init__(self):
        # define arguments and how to validate them
        self.reqparse = reqparse.RequestParser()
        self.reqparse.add_argument('title', type=str, required=True,
        help = 'No title provided', location='json')
        self.reqparse.add_argument('description', type=str, default ='',location = 'json')
        self.reqparse.add_argument('interval', type=list, default=[0], location='json')
        self.reqparse.add_argument('amount', type =int, default =1000, location='json')
        super(OrderListAPI,self).__init__()

    def get(self):
        return {'orders':[marshal(order,order_fields) for order in orders]}

    def post(self):
        args = self.reqparse.parse_args()
        order = {
            'id': orders[-1]['id'] + 1,
            'title': args['title'],
            'description':args['description'],
            'interval':args['interval'],
            'amount': args['amount']
        }
        orders.append(order)
        return {'order': marshal(order, order_fields)}, 201

class OrderAPI(Resource):
    # add this once login is implemented:
    # decorators = [auth.login_required]

    def __init__(self):
        # define arguments and how to validade them
        self.reqparse = reqparse.RequestParser()
        self.reqparse.add_argument('title', type = str, required = True,
        help = 'No title provided', location = 'json')
        self.reqparse.add_argument('description', type = str, default='', location='json')
        # the line below spells trouble, plz debug it
        self.reqparse.add_argument('interval', type = list, default=[0], location='json')
        self.reqparse.add_argument('amount', type = int, location='json')
        super(OrderAPI, self).__init__()


    # the next funtions will have calls to database in the future
    def get(self, id):
        order = [order for order in orders if order['id'] == id]
        if len(order) == 0:
            abort(404)
        return {'order': marshal(order[0], order_fields)}, 200

    def delete(self, id):
        order = [order for order in orders if order['id'] == id]
        if len(order) == 0:
            abort(404)
        orders.remove(order[0])
        return {'result':True}

    def put(self, id):
        # looking for order with matching id in orders dictionary
        order = [order for order in orders if order['id'] == id]
        if len(order) == 0:
            abort(404)
        order = order[0]
        args = self.reqparse.parse_args()
        # update order in dict
        for k, v in args.items():
            if v is not None:
                order[k] = v
        return {'order': marshal(order, order_fields)}, 201


'''
the add_resource function registers the routes with the
framework using the given endpoint.
'''
api.add_resource(OrderListAPI, '/api/v1.0/orders', endpoint='orders')
api.add_resource(OrderAPI, '/api/v1.0/order/<int:id>', endpoint='order')


if __name__ == '__main__':
    app.run(debug=True)
