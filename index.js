const express = require('express');
const app = express();
const mongo = require('mongoose')
const dotenv = require('dotenv')
const HRVReading = require('./schemas/HRVReading')
const asyncHandler = require('./middleware/async')
const cors = require('cors')
const get_hrv = require('./utils/GetHRV')

dotenv.config({path: './config.env'})

mongo.connect(process.env.MONGO_URI, {
    useNewUrlParser: true,
    useCreateIndex: true,
    useFindAndModify: false,
    useUnifiedTopology: true
})

app.listen(3001, () => {
 console.log("Server running on port 3001");
});

app.use(cors())

app.use(express.json());

app.post("/gethrv", asyncHandler(async (req, res, next) => {
    //compute hrv stats e.g. SDNN, rMSSD, HFPWR, etc. from beat to beat data
    const hrvObj = await get_hrv(req.body.beatToBeat, req.body.date)
    // return the obj even if it doesn't get stored (e.g. duplicate)
    if (hrvObj) res.status(201).json(hrvObj)
    // store the computed obj in our readings database
    if (hrvObj) await HRVReading.create(hrvObj)

}))

app.get("/recent", asyncHandler(async (req, res, next) => {
    //get most recent readings, parameterize how many to get
    //TODO: smoothing, pagination
    const readings = await HRVReading.find({}).sort({createdAt: -1}).limit(500)

    res.status(200).json({
        success: true,
        count: readings.length,
        data: readings
    });
    
}));