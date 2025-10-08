#this is for AES-128
import aes_module

key = b'\x2f\x8a\x4e\x93\xd1\x7c\x5b\xe2\xa6\x19\xf4\x3d\x68\xc7\x0b\x9e'

def galois_multiply(a, b):
    p = 0
    a &= 0xFF
    b &= 0xFF
    for _ in range(8):
        if b & 1:
            p ^= a
        hi_bit_set = a & 0x80
        a = (a << 1) & 0xFF        
        if hi_bit_set:
            a ^= 0x1B
        b >>= 1
    return p & 0xFF

def cipher(input_data, number_of_rounds, key):
        key_schedule = KeyExpansion(key)
        state = input_data
        state = AddRoundKey(state, key_schedule[0:4]) #w[0:4] is the first 3 values of the array w, w is the key_schedule

        for i in range(1, number_of_rounds):
            state = SubBytes(state)
            state = ShiftRows(state)
            state = MixColumns(state)
            state = AddRoundKey(state, key_schedule[4*i:4*i+4]) #w[4*round ... 4*round+3]

        state = SubBytes(state)
        state = ShiftRows(state)
        state = AddRoundKey(state, key_schedule[(4*round):(4*round+4)]) #w[4*round ... 4*round+3]

        return state

def SubBytes(state): #state is a 2d array of bytes(8 bits) (4 rows, and 4 cols)
    for row in range(0, 4):
        for col in range(0, 4):
            s_box_transform(state[row][col])
    return state

def s_box_transform(byte):
    #take a single byte and split into a row/col indice
    row = byte / 0x10 
    col = byte % 0x10

    byte = aes_module.S_BOX[row][col] #byte is now equal to the byte of S_BOX at (row, col)
    return byte

def ShiftRows(state):
    copy_state = state

    #s'(r,c) = s (r, (c+r)mod4) for (0 <= row < 4) and (0 <= col < 4)
    for row in range(0, 4):
        for col in range(0, 4):
            copy_state[row][col] = state[row][(row+col)%4]
    return copy_state

def MixColumns(state):
    temp_column = [0, 0, 0, 0]

    for i in range(0, 4):
        for j in range(0, 4):
            temp_column[j] = state[j][i]
        
        state[0][i] = galois_multiply(temp_column[0], 0x02) ^ galois_multiply(temp_column[1], 0x03) ^ temp_column[2] ^ temp_column[3]
        state[1][i] = temp_column[0] ^ galois_multiply(temp_column[1], 0x02) ^ galois_multiply(temp_column[2], 0x03) ^ temp_column[3]
        state[2][i] = temp_column[0] ^ temp_column[1] ^ galois_multiply(temp_column[2], 0x02) ^ galois_multiply(temp_column[3], 0x03)
        state[3][i] = galois_multiply(temp_column[0], 0x03) ^ temp_column[1] ^ temp_column[2] ^ galois_multiply(temp_column[3], 0x02)

    return state

def AddRoundKey(state, key_schedule): 
    new_state = [row[:] for row in state]
    for c in range(4):
        # Get the word for this column (4 bytes)
        word = key_schedule[c]
        
        # XOR each byte in the column with corresponding byte from word
        for r in range(4):
            new_state[r][c] ^= word[r]
    
    return new_state

def KeyExpansion(key):
    Nk = 4   # Number of 32-bit words in key
    Nr = 10 # Number of rounds

    total_words = 4 * (Nr + 1) 
    key_schedule = []

    # First Nk words are the original key
    for i in range(Nk):
        key_schedule.append([
            key[4*i],
            key[4*i + 1],
            key[4*i + 2],
            key[4*i + 3]
        ])

    for i in range(Nk, total_words):
        temp = key_schedule[i-1][:]  # Copy previous word
        
        if i % Nk == 0:
            # RotWord: rotate left by 1 byte
            temp = RotWord(temp)
            # SubWord: apply S-box to each byte
            temp = SubWord(temp)
            # XOR with round constant
            temp[0] ^= aes_module.R_CON[i // Nk - 1][0]
            temp[1] ^= aes_module.R_CON[i // Nk - 1][1]
            temp[2] ^= aes_module.R_CON[i // Nk - 1][2]
            temp[3] ^= aes_module.R_CON[i // Nk - 1][3]
        
        # XOR with word Nk positions earlier
        new_word = []
        for j in range(4):
            new_word.append(key_schedule[i - Nk][j] ^ temp[j])
        
        key_schedule.append(new_word)
    
    return key_schedule

def RotWord(word):
    return [word[1], word[2], word[3], word[0]]

def SubWord(word):
    result = []
    for byte in word:
        row = byte // 0x10
        col = byte % 0x10
        result.append(aes_module.S_BOX[row][col])
    return result