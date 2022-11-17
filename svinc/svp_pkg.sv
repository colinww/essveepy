///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 12-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// SystemVerilog package that contains all the tools available.
//
// Version History
// ---------------
// 13-Nov-22: Initial version
//
///////////////////////////////////////////////////////////////////////////////

`ifndef __SVP__PKG__SV__
`define __SVP__PKG__SV__

package svp_pkg;

///////////////////////////////////////////////////////////////////////////////
// Random signal generator imports
import "DPI-C" function chandle svp_rng_init();
import "DPI-C" function void svp_rng_free(chandle dat);
import "DPI-C" function real svp_rng_rand();
import "DPI-C" function real svp_rng_randn(chandle dat);
import "DPI-C" function real svp_rng_randn_bnd(chandle dat, real rmin,
                                               real rmax);

/**
 * Class which generates uniform and normally-distributed random variables.
 */
class svpRandom;
  // Generator state
  chandle dat;

  /**
   * Create a generator object and its internal state.
   */
  function new();
    this.dat = svp_rng_init();
  endfunction

  /**
   * Generate an I.I.D. random variable uniformly distributed on [0, 1).
   */
  function real urand();
    return svp_rng_rand();
  endfunction

  /**
   * Generate a normally-distributed random variable.
   */
  function real randn();
    return svp_rng_randn(this.dat);
  endfunction

  /**
   * Generate a normally-distributed random variable with bounds.
   *
   * @param minbnd Minimum random value (inclusive).
   * @param maxbnd Maximum random value (exclusive).
   */
  function real randn_bnd(real minbnd, real maxbnd);
    return svp_rng_randn_bnd(this.dat, minbnd, maxbnd);
  endfunction
endclass  // svpRandom


///////////////////////////////////////////////////////////////////////////////
// Flicker noise generator
import "DPI-C" function chandle svp_rng_flicker_new(real flow, real fhigh,
                                                    real spot_freq,
                                                    real spot_amp, real fs);
import "DPI-C" function void svp_rng_flicker_free(chandle dat);
import "DPI-C" function real svp_rng_flicker_samp(chandle dat);
import "DPI-C" function real svp_rng_flicker_samp_scale(chandle dat,
                                                        real scale);

/**
 * Class which generates 1/f shaped noise.
 */
class svpFlicker;
  // Generator state
  chandle dat;

  /**
   * Initalize a generator.
   *
   * @param flow Lower frequency of the 1/f noise shape model.
   * @param fhigh Upper frequency of the 1/f noise shape model.
   * @param spot_freq Frequency for specifying flicker noise power.
   * @param spot_amp Flicker noise density (/rtHz) at spot frequency.
   * @param fs Sampling frequency.
   */
  function new(real flow, real fhigh, real spot_freq, real spot_amp, real fs);
    this.dat = svp_rng_flicker_new(flow, fhigh, spot_freq, spot_amp, fs);
  endfunction

  /**
   * Destructor, MUST be called in a final begin... end block.
   */
  function void free();
    svp_rng_flicker_free(this.dat);
  endfunction

  /**
   * Generate a sample of flicker noise.
   */
  function real samp();
    return svp_rng_flicker_samp(this.dat);
  endfunction

  /**
   * Generate a sample of flicker noise with a dynamic scale.
   */
  function real samp_scale(real scale);
    return svp_rng_flicker_samp_scale(this.dat, scale);
  endfunction
endclass  // svpFlicker


///////////////////////////////////////////////////////////////////////////////
// HDF5 simulation data dumping

/**
 * High-resolution simulation time data.
 */
typedef struct {
  longint ns;
  real rem;
} svp_sim_time_t;

// HDF5 file handling
import "DPI-C" function chandle svp_hdf5_fopen(string fname);
import "DPI-C" function int svp_hdf5_addsig(chandle clsdat, chandle dat);
import "DPI-C" function int svp_hdf5_fclose(chandle clsdat);
// Dump objects
import "DPI-C" function chandle svp_dstore_svcreate(chandle clsdat, string name,
                                                    int store_type, int width,
                                                    string dtype);
// Data writers
import "DPI-C" function int svp_dstore_write_int8(chandle dat, real simtime,
                                                  input byte dbuf []);
import "DPI-C" function int svp_dstore_write_int16(chandle dat, real simtime,
                                                   input shortint dbuf []);
import "DPI-C" function int svp_dstore_write_int32(chandle dat, real simtime,
                                                  input int dbuf []);
import "DPI-C" function int svp_dstore_write_int64(chandle dat, real simtime,
                                                  input longint dbuf []);
import "DPI-C" function int svp_dstore_write_float64(chandle dat, real simtime,
                                                     input real dbuf []);
import "DPI-C" function int svp_dstore_write_time(chandle dat,
                                                  svp_sim_time_t simtime);


/**
 * Class which wraps an HDF5 data file.
 *
 * Once the object is created, data dumps are instantiated and attached to this
 * file object.
 *
 * NOTE: The close() function MUST be called in a final block. Otherwise, the
 * data may not be fully written, and memory leaks will occur.
 */
class svpDumpFile;
// HDF5 file data
chandle dat;

/**
 * Creates a new data file with the name provided.
 *
 * @param fname Name of file to be created.
 */
function new(string fname);
  this.dat = svp_hdf5_fopen(fname);
endfunction

/**
 * Close the file object. This MUST BE CALLED at the end of the simulation
 * (after all writes have finished).
 */
function void close();
  void'(svp_hdf5_fclose(this.dat));
endfunction

endclass  // svpHdf5File


/**
 * Abstract base class for all data dump objects.
 *
 * For now, this does not do much on top of the extension classes, but it
 * exists in case in the future we need to add more common functionality.
 */
virtual class svpDumpAbc;
  // HDF5 dataset data structure
  chandle dat;

  /**
   * Create new data dump object.
   *
   * @param fobj Instance of an SV-wrapped file object.
   * @param signame Name of signal to be dumped (as it will appear in the file).
   * @param is_async If async, store a timestamp with each data sample.
   * @param width Array data width.
   * @param dtype Name of datatype to be stored.
   *
   * Note that the signame can have hierarchy, e.g. u_top.u_mod.u_foo.mysig.
   * The data file will create the hierarchy according to the dot-delimiter.
   */
  function new();
  endfunction

  function alloc(svpDumpFile fobj, string signame, int is_async, int width,
                 string dtype);
    // Create a new dstore object and pass structure pointer back
    this.dat = svp_dstore_svcreate(fobj.dat, signame, is_async, width, dtype);
    // Register signal with the file
    void'(svp_hdf5_addsig(fobj.dat, this.dat));
  endfunction

endclass  // svpDumpAbc


/**
 * Write raw binary signals.
 *
 * @tparam ASYNC If true, a timestamp is stored with each data sample.
 * @tparam SIGNED Specify whether the data should be interpreted as signed.
 * @tparam WIDTH Number of bits in the data bus.
 *
 */
class svpBitDump #(int ASYNC=0, int SIGNED=1, int WIDTH=1) extends svpDumpAbc;
  // Create local versions of all the different data sizes
  byte val8[1];
  shortint val16[1];
  int val32[1];
  longint val64[1];

  /**
   * Create a new data dump object.
   *
   * @param fobj Instance of opened data dump file.
   * @param signame Name of signal (as it will appear in data file).
   */
  function new(svpDumpFile fobj, string signame);
    // Use the smallest possible datatype that can store the vector
    int status;
    if (8 >= WIDTH) begin
      if (SIGNED) begin
        status = super.alloc(fobj, signame, ASYNC, 1, "char");
      end else begin
        status = super.alloc(fobj, signame, ASYNC, 1, "uchar");
      end
    end else if (16 >= WIDTH) begin
      if (SIGNED) begin
        status = super.alloc(fobj, signame, ASYNC, 1, "sint");
      end else begin
        status = super.alloc(fobj, signame, ASYNC, 1, "usint");
      end
    end else if (32 >= WIDTH) begin
      if (SIGNED) begin
        status = super.alloc(fobj, signame, ASYNC, 1, "int");
      end else begin
        status = super.alloc(fobj, signame, ASYNC, 1, "uint");
      end
    end else begin
      if (SIGNED) begin
        status = super.alloc(fobj, signame, ASYNC, 1, "long");
      end else begin
        status = super.alloc(fobj, signame, ASYNC, 1, "ulong");
      end
    end
  endfunction

  /**
   * Write a data sample.
   *
   * @param dwrite Data to be written.
   */
  function void write(input bit [WIDTH-1:0] dwrite);
    if (8 >= WIDTH) begin
      if (SIGNED) begin
        this.val8[0] = signed'(dwrite);
      end else begin
        this.val8[0] = unsigned'(dwrite);
      end
      void'(svp_dstore_write_int8(super.dat, $realtime, this.val8));
    end else if (16 >= WIDTH) begin
      if (SIGNED) begin
        this.val16[0] = signed'(dwrite);
      end else begin
        this.val16[0] = unsigned'(dwrite);
      end
      void'(svp_dstore_write_int16(super.dat, $realtime, this.val16));
    end else if (32 >= WIDTH) begin
      if (SIGNED) begin
        this.val32[0] = signed'(dwrite);
      end else begin
        this.val32[0] = unsigned'(dwrite);
      end
      void'(svp_dstore_write_int32(super.dat, $realtime, this.val32));
    end else begin
      if (SIGNED) begin
        this.val64[0] = signed'(dwrite);
      end else begin
        this.val64[0] = unsigned'(dwrite);
      end
      void'(svp_dstore_write_int64(super.dat, $realtime, this.val64));
    end
  endfunction

endclass  // svpBitDump


/**
 * Write a 1-D packed array of raw binary signals.
 *
 * @tparam ASYNC If true, a timestamp is stored with each data sample.
 * @tparam SIGNED Specify whether the data should be interpreted as signed.
 * @tparam WIDTH Number of bits in the data bus.
 * @tparam SIZE Packed data width.
 *
 */
class svpBitArrayDump
#(int ASYNC=0, int SIGNED=1, int WIDTH=1, int SIZE=1) extends svpDumpAbc;
  // Create local versions of all the different data sizes
  byte val8[SIZE];
  shortint val16[SIZE];
  int val32[SIZE];
  longint val64[SIZE];

  /**
   * Create a new data dump object.
   *
   * @param fobj Instance of opened data dump file.
   * @param signame Name of signal (as it will appear in data file).
   */
  function new(svpDumpFile fobj, string signame);
    // Use the smallest possible datatype that can store the vector
    int status;
    if (8 >= WIDTH) begin
      if (SIGNED) begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "char");
      end else begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "uchar");
      end
    end else if (16 >= WIDTH) begin
      if (SIGNED) begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "sint");
      end else begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "usint");
      end
    end else if (32 >= WIDTH) begin
      if (SIGNED) begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "int");
      end else begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "uint");
      end
    end else begin
      if (SIGNED) begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "long");
      end else begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "ulong");
      end
    end
  endfunction

  /**
   * Write a data sample.
   *
   * @param dwrite Data to be written.
   */
  function void write(input bit [SIZE-1:0][WIDTH-1:0] dwrite);
    // Data must be copied to a C-compliant (unpacked) array
    int ii = 0;
    if (8 >= WIDTH) begin
      if (SIGNED) begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val8[ii] = signed'(dwrite[ii]);
        end
      end else begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val8[ii] = unsigned'(dwrite[ii]);
        end
      end
      void'(svp_dstore_write_int8(super.dat, $realtime, this.val8));
    end else if (16 >= WIDTH) begin
      if (SIGNED) begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val16[ii] = signed'(dwrite[ii]);
        end
      end else begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val16[ii] = unsigned'(dwrite[ii]);
        end
      end
      void'(svp_dstore_write_int16(super.dat, $realtime, this.val16));
    end else if (32 >= WIDTH) begin
      if (SIGNED) begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val32[ii] = signed'(dwrite[ii]);
        end
      end else begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val32[ii] = unsigned'(dwrite[ii]);
        end
      end
      void'(svp_dstore_write_int32(super.dat, $realtime, this.val32));
    end else begin
      if (SIGNED) begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val64[ii] = signed'(dwrite[ii]);
        end
      end else begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val64[ii] = unsigned'(dwrite[ii]);
        end
      end
      void'(svp_dstore_write_int64(super.dat, $realtime, this.val64));
    end
  endfunction

endclass  // svpBitArrayDump


/**
 * Write integers.
 *
 * @tparam ASYNC If true, a timestamp is stored with each data sample.
 * @tparam T integer datatype (byte, shortint, int, longint).
 *
 */
class svpIntegerDump #(int ASYNC=0, type T=byte) extends svpDumpAbc;
  // Local array storage
  byte val8[1];
  shortint val16[1];
  int val32[1];
  longint val64[1];

  /**
   * Create a new data dump object.
   *
   * @param fobj Instance of opened data dump file.
   * @param signame Name of signal (as it will appear in data file).
   */
  function new(svpDumpFile fobj, string signame);
    // Check type
    int status;
    case ($typename(T))
      "byte": begin
        status = super.alloc(fobj, signame, ASYNC, 1, "char");
      end
      "shortint": begin
        status = super.alloc(fobj, signame, ASYNC, 1, "sint");
      end
      "int": begin
        status = super.alloc(fobj, signame, ASYNC, 1, "int");
      end
      "longint": begin
        status = super.alloc(fobj, signame, ASYNC, 1, "long");
      end
      default: begin
        $error("Datatype %s is invalid!", $typename(T));
      end
    endcase
  endfunction

  /**
   * Write a data sample.
   *
   * @param dwrite Data to be written.
   */
  function void write(input T dwrite);
    // Call correct write method
    case ($typename(T))
      "byte": begin
        this.val8[0] = dwrite;
        void'(svp_dstore_write_int8(super.dat, $realtime, this.val8));
      end
      "shortint": begin
        this.val16[0] = dwrite;
        void'(svp_dstore_write_int16(super.dat, $realtime, this.val16));
      end
      "int": begin
        this.val32[0] = dwrite;
        void'(svp_dstore_write_int32(super.dat, $realtime, this.val32));
      end
      "longint": begin
        this.val64[0] = dwrite;
        void'(svp_dstore_write_int64(super.dat, $realtime, this.val64));
      end
    endcase
  endfunction

endclass  // svpIntegerDump


/**
 * Write an array of integers.
 *
 * @tparam ASYNC If true, a timestamp is stored with each data sample.
 * @tparam T integer datatype (byte, shortint, int, longint).
 * @tparam SIZE array width.
 *
 */
class svpIntegerArrayDump
#(int ASYNC=0, type T=byte, int SIZE=1) extends svpDumpAbc;
  // Local array storage
  byte val8[SIZE];
  shortint val16[SIZE];
  int val32[SIZE];
  longint val64[SIZE];

  /**
   * Create a new data dump object.
   *
   * @param fobj Instance of opened data dump file.
   * @param signame Name of signal (as it will appear in data file).
   */
  function new(svpDumpFile fobj, string signame);
    // Check type
    int status;
    case ($typename(T))
      "byte": begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "char");
      end
      "shortint": begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "sint");
      end
      "int": begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "int");
      end
      "longint": begin
        status = super.alloc(fobj, signame, ASYNC, SIZE, "long");
      end
      default: begin
        $error("Datatype %s is invalid!", $typename(T));
      end
    endcase
  endfunction

  /**
   * Write a data sample.
   *
   * @param dwrite Data to be written.
   */
  function void write(input T dwrite[SIZE]);
    // Call correct write method
    int ii;
    case ($typename(T))
      "byte": begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val8[ii] = dwrite[ii];
        end
        void'(svp_dstore_write_int8(super.dat, $realtime, this.val8));
      end
      "shortint": begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val16[ii] = dwrite[ii];
        end
        void'(svp_dstore_write_int16(super.dat, $realtime, this.val16));
      end
      "int": begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val32[ii] = dwrite[ii];
        end
        void'(svp_dstore_write_int32(super.dat, $realtime, this.val32));
      end
      "longint": begin
        for (ii = 0; SIZE > ii; ii++) begin
          this.val64[ii] = dwrite[ii];
        end
        void'(svp_dstore_write_int64(super.dat, $realtime, this.val64));
      end
    endcase
  endfunction

endclass  // svpIntegerDump


/**
 * Write real numbers.
 *
 * @tparam ASYNC If true, a timestamp is stored with each data sample.
 */
class svpRealDump #(int ASYNC=0) extends svpDumpAbc;
  real dval[1];

  /**
   * Create a new data dump object.
   *
   * @param fobj Instance of opened data dump file.
   * @param signame Name of signal (as it will appear in data file).
   */
  function new(svpDumpFile fobj, string signame);
    int status = super.alloc(fobj, signame, ASYNC, 1, "double");
  endfunction

  /**
   * Write a data sample.
   *
   * @param dwrite Data to be written.
   */
  function void write(input real dwrite);
    this.dval[0] = dwrite;
    void'(svp_dstore_write_float64(super.dat, $realtime, this.dval));
  endfunction
endclass  //svpRealDump


/**
 * Write an array of real numbers.
 *
 * @tparam ASYNC If true, a timestamp is stored with each data sample.
 * @tparam T integer datatype (byte, shortint, int, longint).
 * @tparam SIZE array width.
 */
class svpRealArrayDump #(int ASYNC=0, int SIZE=1) extends svpDumpAbc;

  /**
   * Create a new data dump object.
   *
   * @param fobj Instance of opened data dump file.
   * @param signame Name of signal (as it will appear in data file).
   */
  function new(svpDumpFile fobj, string signame);
    int status = super.alloc(fobj, signame, ASYNC, SIZE, "double");
  endfunction

  /**
   * Write a data sample.
   *
   * @param dwrite Data to be written.
   */
  function void write(input real dwrite[SIZE]);
    void'(svp_dstore_write_float64(super.dat, $realtime, dwrite));
  endfunction

endclass // svpRealArrayDump


/**
 * Write high-resolution timestamps.
 *
 */
class svpTimeDump extends svpDumpAbc;
  longint ns[1];

  /**
   * Create a new data dump object.
   *
   * @param fobj Instance of opened data dump file.
   * @param signame Name of signal (as it will appear in data file).
   */
  function new(svpDumpFile fobj, string signame);
    int status = super.alloc(fobj, signame, 0, 1, "time");
  endfunction

  /**
   * Write a data sample.
   *
   * @param dwrite Data to be written.
   */
  function void write(input svp_sim_time_t dwrite);
    this.ns[0] = dwrite.ns;
    void'(svp_dstore_write_int64(super.dat, dwrite.rem, this.ns));
  endfunction
endclass  //svpRealDump


endpackage  // svp_pkg
`endif
