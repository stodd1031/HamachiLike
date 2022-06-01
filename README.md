# HamachiLike

A program that mimics what hamachi does using P2P instead of a block of purchased IP adresses and managing routing servers.

Video explination: https://www.youtube.com/watch?v=T84478DITZc

"Final" file is dhcpfinal.c and .h. Most of the other files are learning and testing files.

Inititally tried using DHCP messages to secure a secodnary private IP on an interface. I couldn't get this to work so my next plan is to use an IP that will not be on the subnet and use ARP messages to insure single usage of the IP. I have yet to finish this part of the project.

I plan to clean up project. Once working I plan to encrypt data along the P2P tunnel(s).
