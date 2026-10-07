import unittest

from payobc_link import FrameDecoder, encode_frame


COMMAND = bytes.fromhex("10 10 c0 00 00 06 20 00 01 00 00 00 01")
FRAME = bytes.fromhex("1a cf fc 1d 00 0d 10 10 c0 00 00 06 20 00 01 00 00 00 01 9b ba")


class PayloadLinkTest(unittest.TestCase):
    def test_known_vector(self):
        self.assertEqual(encode_frame(COMMAND), FRAME)

    def test_split_and_resync(self):
        decoder = FrameDecoder()
        self.assertEqual(decoder.feed(b"noise" + FRAME[:8]), [])
        self.assertEqual(decoder.feed(FRAME[8:]), [COMMAND])

    def test_bad_crc_is_dropped(self):
        bad = bytearray(FRAME)
        bad[-1] ^= 0xFF
        self.assertEqual(FrameDecoder().feed(bytes(bad)), [])


if __name__ == "__main__":
    unittest.main()
