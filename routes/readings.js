const express = require('express')
const HRVReading = require('../schemas/HRVReading')
const RHRReading = require('../schemas/RHRReading')
const asyncHandler = require('../middleware/async')
const get_hrv = require('../utils/GetHRV')
const moment = require('moment')
const app = express.Router()

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

app.get("/hrv/mostrecent", asyncHandler(async (req, res, next) => {

    let mostRecentHRV = await HRVReading.findOne().sort({ createdAt: -1 })
    res.status(200).json(mostRecentHRV)

}))

app.post("/rhr", asyncHandler(async (req, res, next) => {
    const saved = await RHRReading.create({ restingHeartRate: req.body.heartrate, createdAt: req.body.timestamp })
    res.status(201).json(saved)
}))

app.get("/rhr", asyncHandler(async (req, res, next) => {
    //get most recent readings, parameterize how many to get
    //TODO: smoothing, pagination
    let startDate = moment().subtract(6, 'week').toDate();


    const readings = await RHRReading.find({createdAt: { $gte: startDate } }).sort({ createdAt: -1 })

    res.status(200).json({
        success: true,
        count: readings.length,
        data: readings
    });

}));

app.get("/rhr/mostrecent", asyncHandler(async (req, res, next) => {

    let mostRecentRHR = await RHRReading.findOne().sort({ createdAt: -1 })
    res.status(200).json(mostRecentRHR)

}));

app.get("/readiness/:fromNowInt/:fromNowUnit", asyncHandler(async (req, res, next) => {
    
    let fromNow = moment().subtract(req.params.fromNowInt, req.params.fromNowUnit).toDate()

    //compare to recent 6 weeks?
    let startDate = moment().subtract(6, 'week').toDate();

    //last year?
    // let startDate = moment().subtract(1, 'year').toDate();

    let currentRHR
    const RHRsfromNow = await RHRReading.find({ createdAt: {$gte: fromNow}}).sort({ createdAt: 1 })
    if (RHRsfromNow[0]) {
        currentRHR = RHRsfromNow.reduce((totalObj, rhr) => {
            return {
                createdAt: rhr.createdAt,
                id: rhr.id,
                restingHeartRate: totalObj.restingHeartRate + rhr.restingHeartRate
            }
        })
        currentRHR = {
            createdAt: currentRHR.createdAt,
            id: currentRHR.id,
            restingHeartRate: currentRHR.restingHeartRate / RHRsfromNow.length
        }
    }
    else {
        currentRHR = await RHRReading.findOne().sort({ createdAt: -1 })
    }

    const gtRHR = await RHRReading.find({ restingHeartRate: { $gte: currentRHR.restingHeartRate }, createdAt: { $gte: startDate } }).count()
    const ltRHR = await RHRReading.find({ restingHeartRate: { $lt: currentRHR.restingHeartRate }, createdAt: { $gte: startDate } }).count()
    rhrPercentile = gtRHR / (gtRHR + ltRHR) //lower is better

    
    let currentHRV 
    const HRVsfromNow = await HRVReading.find({ createdAt: { $gte: fromNow } }).sort({ createdAt: 1 })
    console.log("HRVsFromNow", HRVsfromNow)
    if (HRVsfromNow[0]) {
        currentHRV = HRVsfromNow.reduce((totalObj, hrv) => {
            return {
                createdAt: hrv.createdAt,
                id: hrv.id,
                SDNN: totalObj.SDNN + hrv.SDNN,
                rMSSD: totalObj.rMSSD + hrv.rMSSD,
                HFPWR: totalObj.HFPWR + hrv.HFPWR
            }
        })
        currentHRV = {
            createdAt: currentHRV.createdAt,     
            id: currentHRV.id,       
            SDNN: currentHRV.SDNN / HRVsfromNow.length,
            rMSSD: currentHRV.rMSSD / HRVsfromNow.length,
            HFPWR: currentHRV.HFPWR / HRVsfromNow.length
        }
    }
    else {
        currentHRV = await HRVReading.findOne().sort({ createdAt: -1 })
    }
    console.log("currentHRV", currentHRV)

    const gtSDNN = await HRVReading.find({ SDNN: { $gt: currentHRV.SDNN }, createdAt: { $gte: startDate } }).count()
    const ltSDNN = await HRVReading.find({ SDNN: { $lte: currentHRV.SDNN }, createdAt: { $gte: startDate } }).count()
    sdnnPercentile = ltSDNN / (gtSDNN + ltSDNN) //higher is better


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
            createdAt: currentRHR.createdAt,
            id: currentRHR.id,
            units: 'bpm'
        },
        {
            label: "Heart Rate Variability (HF Power) Readiness",
            percentile: hfpwrPercentile,
            currentValue: currentHRV.HFPWR,
            createdAt: currentHRV.createdAt,
            id: currentHRV.id + '2',
            units: ''
        },
        {
            label: "Heart Rate Variability (rMSSD) Readiness",
            percentile: rMSSDPercentile,
            currentValue: currentHRV.rMSSD,
            createdAt: currentHRV.createdAt,
            id: currentHRV.id + '1',
            units: 'ms'
        },
        {
            label: "Heart Rate Variability (SDNN) Readiness",
            percentile: sdnnPercentile,
            currentValue: currentHRV.SDNN,
            createdAt: currentHRV.createdAt,
            id: currentHRV.id,
            units: 'ms'

        }
    ])
}))

module.exports = app