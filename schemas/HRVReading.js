const mongo = require('mongoose')

const HRVReading_Schema = new mongo.Schema({
    NNtoRR: {type: Number, trim: true },
    AVNN: {type: Number, trim: true },
    SDNN: {type: Number, trim: true },
    SDANN: {type: Number, trim: true },
    SDNNIDX: {type: Number, trim: true },
    rMSSD: {type: Number, trim: true },
    pNN50: {type: Number, trim: true },
    TOTPWR: {type: Number, trim: true },
    ULFPWR: {type: Number, trim: true },
    VLFPWR: {type: Number, trim: true },
    LFPWR: {type: Number, trim: true },
    HFPWR: {type: Number, trim: true },
    LFtoHF: {type: Number, trim: true },

    createdAt: {type: Date, default: Date.now}

    
})

module.exports = mongo.model('HRVReading', HRVReading_Schema)