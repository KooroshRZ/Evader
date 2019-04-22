# Evader project


It's a exe packer which will encrypt your PE exe input file and add it as resource to the end of the output new exe file

the encryption key size and complexity can be given as input

Usage
----------
just run this command to make you file packed

.. code-block:: batch

  packer.exe <input-path> <output-path> <key-size> <start-ascii> <end-ascii>
  
The complexity of encryption key will be determined by <start-ascii> and <end-ascii>
  
for example this command will lead to keys from AAAA to ZZZZ

.. code-block:: batch

  packer.exe <input-path> <output-path> 4 65 90




----------
tnx for https://www.codeproject.com/Articles/5035/How-to-Write-a-Simple-Packer-Unpacker-with-a-Self
tnx for https://www.youtube.com/watch?v=bQWRW0VUXR4
