const express = require('express');
const app = express();
const cors = require('cors')
const connectDB = require('./utils/db')

connectDB()

app.use(cors())
app.use(express.json());
app.get('/', (req, res) => {
    res.json({ msg: 'Welcome to HRV Dashboard API!'})
})

app.use('/api/auth', require('./routes/auth'))
app.use('/api/readings', require('./routes/readings'))
app.use('/api/users', require('./routes/users'))

const PORT = process.env.PORT || 3001

app.listen(PORT, () => console.log(`Server started on port ${PORT}`))


