# Evader
========
 
It's an exe packer (and a crypter) which will encrypt your PE exe input file and add it as resource to the end of the output new exe file
The encryption key size and complexity can be given as input

# Usage of packer (Encryption)
-----------------

    packer.exe <input-path> <output-path> <key-size> <start-ascii> <end-ascii>
  
The complexity of encryption key will be determined by <start-ascii> and <end-ascii>
  
for example this command will lead to keys from AAAA to ZZZZ

    packer.exe <input-path> <output-path> 4 65 90
    
# The Decryption part
-------------------
Decryption isn't like other packers stub and it's based on bruteforcing and examining each character to retrieve the encryption key! why!?

And after retrieving key the encrypted payload will be decrypted and will be run directly inside memory and in form of char[] whcih is contained each byte of the main PE file!

# Credits
 https://www.codeproject.com/Articles/5035/How-to-Write-a-Simple-Packer-Unpacker-with-a-Self
 https://www.youtube.com/watch?v=bQWRW0VUXR4
