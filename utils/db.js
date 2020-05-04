const mongoose = require('mongoose') //(13)
const dotenv = require('dotenv')
dotenv.config({ path: './config.env' })

const connectDB = async () => { 
    try { 
        await mongoose
        .connect(process.env.MONGO_URI, {
            useNewUrlParser: true,
            useCreateIndex: true,
            useFindAndModify: false,
            useUnifiedTopology: true
        })

        console.log('MongoDB Connected...')
        
    } catch (err) {
        console.error(err.message)
        process.exit(1) // will exit with failure
    }
}


module.exports = connectDB // (14) --> bring into server.js