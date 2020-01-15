const express = require('express')
const bodyParser = require('body-parser')
const { Client } = require('@elastic/elasticsearch')
const port = 9000
const app = express()
const es = new Client({
	node: 'http://localhost:9200',
	auth: {
		username: process.env.ESTC_USER,
		password: process.env.ESTC_PASS
	},
	keepAlive: true
})

app.use(bodyParser.json())
app.use(bodyParser.urlencoded({extended: false}))

app.post('/weather/_doc/', async (req, res) => {
	console.log(req.body)

	let doc = req.body
	doc.sample_time = Date.now()

	try {
		const result = await es.index({
			index: 'weather',
			body: doc
		})
		console.log(result.statusCode, result.body.result)
	} catch (error) {
		console.log('error:', Object.keys(error))
		console.log('      ', error.name)
		console.log('      ', error.meta)
	}
	res.send('ok')

})

console.log(`started on port ${port}`)

app.listen(port)
