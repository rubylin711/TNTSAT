///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

// classes to manipulate bit strings
// MSB first (MPEG and VC1 style)
#pragma once

class bit_string_reader{
private:
int CurrentByte;
int CurrentBit;
unsigned char *bytes;
int bytes_len;

public:
void init(unsigned char *data,int len)
{
CurrentByte=0;
CurrentBit=7;
bytes=data;
bytes_len=len;
}

int new_bit() // returns one next bit from the stream
{
int ret;
// check end of string error
if(CurrentByte>bytes_len-1) return -1; // error end of string
ret=(((bytes[CurrentByte])>>CurrentBit) & 1);
if(CurrentBit>0)
   CurrentBit--;
else
    {
    CurrentByte++;
    CurrentBit=7;
    }
return ret;
}

int get_bits(int n) // reads n bits positive integer (up to 31 bits) from string
{
    // this function can read more than 32 bits
    // but after reading n > 32 returned data may be invalid
    // may be used to skip more than 32 bits at a time
    int decoded_bits=0;
    int i, b;

    for(i=0;i<n;i++)
    {
        b=new_bit();
            if(b==-1) // error check
            return -1; // error
    decoded_bits=((decoded_bits<<1) | b);
    }//end for
return decoded_bits;
}

int bits_left() const // how many bits left in buffer?
{
return (bytes_len-CurrentByte)*8-(7-CurrentBit);
}

};




//////////////////////////////////////////////////////////////////////

class bit_string_writer{
private:
int CurrentByte;
int CurrentBit;
unsigned char *bytes;
int maximum_len;

public:
void init(unsigned char *data,int max_len)
{
CurrentByte=0;
CurrentBit=7;
bytes=data;
maximum_len=max_len;
int i;
// init output biffer with 0 s
for(i=0;i<max_len;i++)
    bytes[i]=0;
}

int write_bit(int b) // writes bit to the stream
{
unsigned char r;
// check end of string error
if(CurrentByte>maximum_len-1) return -1; // error end of buffer

r=((b & 1)<<CurrentBit);
bytes[CurrentByte]= (bytes[CurrentByte] | r);
if(CurrentBit>0)
   CurrentBit--;
else
    {
    CurrentByte++;
    if(CurrentByte<maximum_len)
       bytes[CurrentByte]=0;  // clear next byte
    CurrentBit=7;
    }
return b;
}

int put_bits(int value, int n) // writes n bits positive integer (up to 31 bits) to output string
{
    int i, b;
    int ret;

    for(i=n-1;i>=0;i--)
    {
        b= ((value>>i) & 1);
        ret= write_bit(b);
            if(ret==-1) // error check
               return -1; // error
    }//end for
return 1;  // succeeded
}

int bits_left() const // how many bits left in buffer?
{
return ((maximum_len-1)-CurrentByte)*8-(7-CurrentBit);
}

int bits_stored() const
{
return (CurrentByte*8+(7-CurrentBit));
}

int bytes_stored() const
{
int ret;
ret=CurrentByte;
if(CurrentBit<7) ret++; // few extra bits

return ret;
}

};

