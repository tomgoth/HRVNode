const express = require('express');
const app = express();
const mongo = require('mongoose')
const dotenv = require('dotenv')
const HRVReading = require('./schemas/HRVReading')
const RHRReading = require('./schemas/RHRReading')
const asyncHandler = require('./middleware/async')
const cors = require('cors')
const get_hrv = require('./utils/GetHRV')
const moment = require('moment')

dotenv.config({ path: './config.env' })

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

app.post("/hrv", asyncHandler(async (req, res, next) => {
    //compute hrv stats e.g. SDNN, rMSSD, HFPWR, etc. from beat to beat data
    const hrvObj = await get_hrv(req.body.beatToBeat, req.body.date)
    // return the obj even if it doesn't get stored (e.g. duplicate)
    if (hrvObj) res.status(201).json(hrvObj)
    // store the computed obj in our readings database
    //TODO add user info
    if (hrvObj) await HRVReading.create(hrvObj)

}))

app.get("/hrv", asyncHandler(async (req, res, next) => {
    //get most recent readings, parameterize how many to get
    //TODO: smoothing, pagination
    const readings = await HRVReading.find({}).sort({ createdAt: -1 }).limit(100)

    res.status(200).json({
        success: true,
        count: readings.length,
        data: readings
    });

}));

app.post("/rhr", asyncHandler(async (req, res, next) => {
    const saved = await RHRReading.create({ restingHeartRate: req.body.heartrate, createdAt: req.body.timestamp })
    res.status(201).json(saved)
}))

app.get("/rhr", asyncHandler(async (req, res, next) => {
    //get most recent readings, parameterize how many to get
    //TODO: smoothing, pagination
    const readings = await RHRReading.find({}).sort({ createdAt: -1 }).limit(100)

    res.status(200).json({
        success: true,
        count: readings.length,
        data: readings
    });

}));

app.get("/readiness", asyncHandler(async (req, res, next) => {

    //limit to recent 6 weeks?
    let startDate = moment().subtract(6, 'week').toDate();

    const currentRHR = await RHRReading.findOne().sort({ createdAt: -1 })
    const gtRHR = await RHRReading.find({ restingHeartRate: { $gte: currentRHR.restingHeartRate }, createdAt: { $gte: startDate } }).count()
    const ltRHR = await RHRReading.find({ restingHeartRate: { $lt: currentRHR.restingHeartRate }, createdAt: { $gte: startDate } }).count()
    rhrPercentile = gtRHR / (gtRHR + ltRHR) //lower is better

    const currentHRV = await HRVReading.findOne().sort({ createdAt: -1 })
    const gtRMSSD = await HRVReading.find({ rMSSD: { $gt: currentHRV.rMSSD }, createdAt: { $gte: startDate } }).count()
    const ltRMSSD = await HRVReading.find({ rMSSD: { $lte: currentHRV.rMSSD }, createdAt: { $gte: startDate } }).count()
    rMSSDPercentile = ltRMSSD / (gtRMSSD + ltRMSSD) //higher is better

    const gtHFPWR = await HRVReading.find({ HFPWR: { $gt: currentHRV.HFPWR }, createdAt: { $gte: startDate } }).count()
    const ltHFPWR = await HRVReading.find({ HFPWR: { $lte: currentHRV.HFPWR }, createdAt: { $gte: startDate } }).count()
    hfpwrPercentile = ltHFPWR / (gtHFPWR + ltHFPWR) //higher is better


    res.status(200).json([
        {
            label: "Resting Heart Rate Readiness",
            percentile: rhrPercentile,
            currentValue: currentRHR.restingHeartRate,
            createdAt: currentRHR.createdAt
        },
        {
            label: "Heart Rate Variability (rMSSD) Readiness",
            percentile: rMSSDPercentile,
            currentValue: currentHRV.rMSSD,
            createdAt: currentHRV.createdAt
        },
        {
            label: "Heart Rate Variability (HF Power) Readiness",
            percentile: hfpwrPercentile,
            currentValue: currentHRV.HFPWR,
            createdAt: currentHRV.createdAt
        }
    ])
}))