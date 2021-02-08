const mongo = require('mongoose')

const HRVReading_Schema = new mongo.Schema({
    user: {
        type: mongo.Schema.Types.ObjectId,
        ref: 'user'
    },
    NNtoRR: { type: Number },
    AVNN:  { type: Number },
    SDNN:  { type: Number },
    SDANN:  { type: Number },
    SDNNIDX:  { type: Number },
    rMSSD:  { type: Number },
    pNN50:  { type: Number },
    TOTPWR:  { type: Number },
    ULFPWR:  { type: Number },
    VLFPWR:  { type: Number },
    LFPWR:  { type: Number },
    HFPWR:  { type: Number },
    LFtoHF:  { type: Number },
    createdAt: { type: Date, default: Date.now },
    isECG: {type: Boolean }
})

HRVReading_Schema.index({ "user": 1, "createdAt": 1 }, { unique: true })

module.exports = mongo.model('HRVReading', HRVReading_Schema)