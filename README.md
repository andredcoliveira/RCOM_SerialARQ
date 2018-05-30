# RCOM_Lab1
Data-Link Protocol implementing Stop-and-Wait ARQ Protocol with an API for communication over Serial Port. This could be, for example, the transfer of a binary file, keeping as much info as we can on it (name, permissions, dates, ...).

Usage:
  ```
  make
  ./transfer /dev/ttsy0 <file_to_be_transmitted>
  'RX' or 'TX' (receiver mode or transmitter mode)
  ```
