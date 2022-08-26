import json

def main():
    file = open("LEGO_MMOG.exe", "rb")
    data = file.read()
    file.close()
    
    bytesToSkip = []
    
    mostRecentString = ""
    
    commands = {
        
    }
    
    done = False
    
    for x in range(len(data) - 0x00f8d22c): # magic numbers are poggers
        currentPos = x + 0x00f8d22c
        byte = data[currentPos]
        
        if currentPos in bytesToSkip:
            bytesToSkip.remove(currentPos)
            continue
        
        if not byte == 0x00:
            index = 0
            hitNull = False
            hitTwice = False
            currentString = ""
            while True:
                currentByte = data[currentPos + index]
                
                if currentByte == 0x00:
                    if hitNull:
                        if not hitTwice:
                            if "func" in currentString:
                                done = True
                                break
                            
                            print(currentString)
                            
                            for x in range(len(currentString)):
                                bytesToSkip.append(currentPos + x)
                        hitTwice = True
                        
                        bytesToSkip.append(currentPos + index)
                        
                    hitNull = True
                else:
                    if hitTwice:
                        break
                    hitNull = False
                    
                currentString += chr(currentByte)
                
                index += 1
                
            if done:
                break
            
            continue

if __name__ == "__main__":
    main()
