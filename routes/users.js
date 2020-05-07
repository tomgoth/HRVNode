const express = require('express'); //(5) --> routes in file
const router = express.Router();
const { check, validationResult } = require('express-validator'); // (19) allows us to set checks on certain inputs
const bcrypt = require('bcryptjs'); // their methods return a promise
const jwt = require('jsonwebtoken');
const User = require('../schemas/User') // (17) -->add middleware to server



// @route   POST api/users
// @desc    Register a user
// @access  Public
router.post('/', [ //(6) -> contacts
    check('name', 'Please add a name').not().isEmpty(), // (20) validators - they just set the checks
    check('email', 'Please include a valid email').isEmail(),
    check('password', 'Please enter a password with 6 or more characters').isLength({ min: 6 })
], 
async (req, res) => {
    
    const errors = validationResult(req) 

    if(!errors.isEmpty()) {
        return res.status(400).json({ errors: errors.array() }) // (21) bad request, the user did not send the required fields with appropriate data
    }

    const { name, email, password } = req.body // (22) destructor the request body

    try { 
        let user = await User.findOne({ email }) // (23) findOne is a method from mongoose

        if(user) {
            console.log("User already exists", user)
            return res.status(400).json({ msg: 'User already exists' })
        }

        user = new User({ // (24) if the user does not exist then we are going to create the new user object
            name,
            email,
            password // plain text password
        })

        //(25)
        // before we save to db we need to encrypt password - returns a promise
        const salt = await bcrypt.genSalt(10) 
        // hash password - takes in the plain text password with the salt 
        user.password = await bcrypt.hash(password, salt)
        // save password - returns a promise -> check db
        await user.save()


        // JWT
            // header, payload, signature
            // payload is the objecy I want to send 
            // then we sign the jwt
                // pass in the payload, the secret (set in the global variables), and an ojbect of options (expiresIn (1hr), cb with err and token itself)


        // payload is the object I want to send in the token - all i want to send is the user.id and access the contacts of that user
        const payload = {
            user: {
                id: user.id
            }
        }

        jwt.sign(payload, process.env.jwtSecret, { expiresIn: 360000 }, (err, token) => {
            if (err) {
                throw err
            }
            // will just return the token
            res.json({ token })
        })

    } catch (err) {
        console.error(err.message)
        res.status(500).send('Server Error')
    }

})

module.exports = router;