key = b'\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c'

def galois_multi(byte, multiplier):
    result = 0

    for i in range(8):
        if (multiplier & 0x01):
            result ^= byte

        high_bit_set = byte & 0x80
        byte <<= 1

        if high_bit_set:
            byte ^= 0x1B

        multiplier >>= 1

    return result

def cipher(input_data, number_of_rounds, key):
    key_schedule = KeyExpansion(key)
    state = input_data
    state = AddRoundKey(state, key_schedule[0:4])

    for i in range(1, number_of_rounds):
        state = SubBytes(state)
        state = ShiftRows(state)
        state = MixColumns(state)
        state = AddRoundKey(state, key_schedule[4*i:4*i+4])  

    state = SubBytes(state)
    state = ShiftRows(state)
    state = AddRoundKey(state, key_schedule[4*number_of_rounds:4*number_of_rounds+4])  

    return state

def SubBytes(state):
    for row in range(4):
        for col in range(0, 4):
            transformed_row = state[row][col] // 0x10
            transformed_col = state[row][col] % 0x10
            state[row][col] = S_BOX[state[transformed_row][transformed_col]] 
    return state
'''void AES::sub_bytes(uint8_t (&state)[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            s_box_transform(state[i][j]);
        }
    }
}'''
'''void AES::s_box_transform(uint8_t &byte) {
    uint8_t row = byte / 0x10;
    uint8_t column = byte % 0x10;

    byte = s_box[row][column];
}'''

def ShiftRows(state):
    original_state = state

    for i in range(4):
        state[1][i] = original_state[1][(i+1)%4]
        state[2][i] = original_state[2][(i+2)%4]
        state[3][i] = original_state[3][(i+3)%4]
'''void AES::shift_rows(uint8_t (&state)[4][4]) {
    // Copy state to original_state
    uint8_t original_state[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            original_state[i][j] = state[i][j];
        }
    }

    // Row 1: Shift left by 1
    for (int i = 0; i < 4; i++) {
        state[1][i] = original_state[1][(i + 1) % 4];
    }

    // Row 2: Shift left by 2
    for (int i = 0; i < 4; i++) {
        state[2][i] = original_state[2][(i + 2) % 4];
    }

    // Row 3: Shift left by 3
    for (int i = 0; i < 4; i++) {
        state[3][i] = original_state[3][(i + 3) % 4];
    }
}'''

def MixColumns(state):
    temp_col = [[],[],[],[]]

    for i in range(4):
        for j in range(4):
            temp_col[j] = state[j][i]
        
        state[0][i] = galois_multi(temp_col[0], 0x02) ^ galois_multi(temp_col[1], 0x03) ^ temp_col[2] ^ temp_col[3]
        state[1][i] = temp_col[0] ^ galois_multi(temp_col[1], 0x02) ^ galois_multi(temp_col[2], 0x03) ^ temp_col[3]
        state[2][i] = temp_col[0] ^ temp_col[1] ^ galois_multi(temp_col[2], 0x02) ^ galois_multi(temp_col[3], 0x03)
        state[3][i] = galois_multi(temp_col[0], 0x03) ^ temp_col[1] ^ temp_col[2] ^ galois_multi(temp_col[3], 0x02)
'''void AES::mix_columns(uint8_t (&state)[4][4]) {
    // make a copy of the existing state
    uint8_t temp_col[4];

    // use galois matrix multiplcation to find new indicies
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp_col[j] = state[j][i];
        }
        
        // change the state as per the algorithm formula
        state[0][i] = galois_multi(temp_col[0], 0x02) ^ galois_multi(temp_col[1], 0x03) ^ temp_col[2] ^ temp_col[3];
        state[1][i] = temp_col[0] ^ galois_multi(temp_col[1], 0x02) ^ galois_multi(temp_col[2], 0x03) ^ temp_col[3];
        state[2][i] = temp_col[0] ^ temp_col[1] ^ galois_multi(temp_col[2], 0x02) ^ galois_multi(temp_col[3], 0x03);
        state[3][i] = galois_multi(temp_col[0], 0x03) ^ temp_col[1] ^ temp_col[2] ^ galois_multi(temp_col[3], 0x02);
    }
}'''

def AddRoundKey(state, round_key): 
    for i in range(4):
        for j in range(4):
            state[j][i] ^= round_key[(i*4) + j]
'''void AES::add_round_key(uint8_t (&state)[4][4], uint8_t round_key[16]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[j][i] ^= round_key[(i * 4) + j];
        }
    }
}'''

def KeyExpansion(key_original, key_schedule):
    i = 0

    #Nk (key length) = 4, Nr (num of rounds) = 10

    for i in range(16):
        key_schedule[i] = key_original[i]

    total_words = (11)*(4) #for AES-128 
    
    for i in range(4, total_words):
        temp = [[], [], [], []]
        for j in range(4):
            temp[j] = key_schedule[(i-1) * 4 + j]
        if (i % 4 == 0):
            RotWord(temp)
            SubWord(temp)

            temp[0] ^= R_CON[i / 4 - 1][0]
            temp[1] ^= R_CON[i / 4 - 1][1]
            temp[2] ^= R_CON[i / 4 - 1][2]
            temp[3] ^= R_CON[i / 4 - 1][3]
        for j in range(4):
            key_schedule[i * 4 + j] = key_schedule[(i - 4) * 4 + j] ^ temp[j]
'''void AES::key_expansion() {
    int i = 0;

    // copy original key into key schedule
    for (i = 0; i < key_word_size * 4; i++) {
        key_schedule[i] = key_original[i];
    }

    // create the other rounds keys
    // e.g 128 --> 44 words, 10 rounds, 176 bytes
    int total_words = (round_num + 1) * key_word_size;
    for (i = key_word_size; i < total_words; i++) {
        // get previous key word
        uint8_t temp[4];
        for (int j = 0; j < 4; j++) {
            temp[j] = key_schedule[(i - 1) * 4 + j];
        }

        // apply key schedule
        // NOTE AES-256 has special subword case, shown in else-if statement
        if (i % key_word_size == 0) {
            rot_word(temp);
            sub_word(temp);

            temp[0] ^= r_con[i / key_word_size - 1][0];
            temp[1] ^= r_con[i / key_word_size - 1][1];
            temp[2] ^= r_con[i / key_word_size - 1][2];
            temp[3] ^= r_con[i / key_word_size - 1][3];

        } else if (key_word_size > 6 && i % key_word_size == 4) {
            sub_word(temp);

        }

        // XOR current key with previous key's first word
        for (int j = 0; j < 4; j++) {
            key_schedule[i * 4 + j] = key_schedule[(i - key_word_size) * 4 + j] ^ temp[j];
        }
    }
}'''

def RotWord(word):
    word[0], word[1], word[2], word[3] = word[1], word[2], word[3], word[0]

def SubWord(word):
    for i in range(4):
        transformed_row = word[i] // 0x10
        transformed_col = word[i] % 0x10

        word[i] = S_BOX[transformed_row][transformed_col]
'''void AES::sub_word(uint8_t (&key)[4]) {
    for (int i = 0; i < 4; i++) {
        s_box_transform(key[i]);
    }
}'''
'''void AES::s_box_transform(uint8_t &byte) {
    uint8_t row = byte / 0x10;
    uint8_t column = byte % 0x10;

    byte = s_box[row][column];
}'''


S_BOX = [
    # 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
    [0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76],  # 0
    [0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0],  # 1
    [0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15],  # 2
    [0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75],  # 3
    [0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84],  # 4
    [0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF],  # 5
    [0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8],  # 6
    [0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2],  # 7
    [0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73],  # 8
    [0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB],  # 9
    [0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79],  # A
    [0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08],  # B
    [0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A, 0x70],  # C
    [0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, 0xE1],  # D
    [0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF, 0x8C],  # E
    [0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16, 0x63]   # F
]

R_CON = [
    [0x01, 0x00, 0x00, 0x00],
    [0x02, 0x00, 0x00, 0x00],
    [0x04, 0x00, 0x00, 0x00],
    [0x08, 0x00, 0x00, 0x00],
    [0x10, 0x00, 0x00, 0x00],
    [0x20, 0x00, 0x00, 0x00],
    [0x40, 0x00, 0x00, 0x00],
    [0x80, 0x00, 0x00, 0x00],
    [0x1B, 0x00, 0x00, 0x00],
    [0x36, 0x00, 0x00, 0x00]
]