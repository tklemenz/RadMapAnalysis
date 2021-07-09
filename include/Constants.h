#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "Utility.h"
#include <map>

/// This namespace holds some useful constants
namespace constants
{
  /// This gain correction is taken from the single SiPM lab measurement with 15ns ToT, pulled to 15ns
  /// Start with 0 for convenience. Channels count from 1 to 16.
  const std::map<Padiwa, std::vector<Float_t>> padiwaGainCorr{
    { Padiwa::p1500_0, {0, 0.993956, 0.947748, 1.00321, 0.94972, 0.998419, 0.956976, 1.01428, 0.990555, 1.00563, 0.947146, 1.07335, 0.983669, 1.00338, 0.975554, 1.0446, 0.945965} },
    { Padiwa::p1500_1, {0, 0.944714, 0.942939, 0.990904, 0.916818, 0.995356, 0.960099, 1.02405, 0.98154, 1.05413, 0.961365, 1.11653, 0.964116, 0.984794, 0.990447, 1.03075, 0.994417} },
    { Padiwa::p1510_0, {0, 0.977523, 0.959283, 0.989304, 0.958931, 0.982616, 0.970987, 0.969084, 0.983486, 1.04562, 0.992069, 1.05032, 0.965812, 0.987799, 0.985607, 1.08823, 1.00406} },
    { Padiwa::p1510_1, {0, 0.784554, 0.781036, 0.807956, 0.758834, 0.860381, 0.806506, 0.827641, 0.823715, 0.890939, 0.82255, 0.949603, 0.848266, 0.866951, 0.854033, 0.913789, 0.870034} },
    { Padiwa::p1520_0, {0, 1.04055, 1.00369, 1.01435, 1.00105, 1.02302, 1.01781, 0.977576, 1.0419, 1.06463, 1.0319, 1.07306, 1.01746, 0.972681, 0.994663, 1.08634, 1.09449} },
    { Padiwa::p1520_1, {0, 1.03476, 1.03128, 0.989076, 0.992776, 1.00963, 0.999363, 1.0192, 1.03511, 1.09152, 1.01686, 1.14649, 1.01658, 0.978441, 1.00529, 1.0982, 1.06285} },
    { Padiwa::p1530_0, {0, 0.968393, 0.939864, 0.994725, 0.955928, 1.01725, 0.997672, 0.975516, 1.01673, 1.01239, 1.00043, 1.11051, 0.956801, 0.989546, 0.9773, 1.06581, 0.992145} },
    { Padiwa::p1530_1, {0, 0.993151, 0.983221, 0.991312, 0.964011, 1.02646, 0.968639, 1.02203, 1.01134, 1.05858, 0.960808, 1.11341, 0.974927, 0.961498, 1.00427, 1.04407, 1.05516} }
  };

  //===========================================================================================================================
  /// Values are from all 0 deg, threshold 100 pion data at config 0 and 4
  /// Just for the fun of it try to calibrate module gain also via padiwa mapping
  /// @todo Still does only work properly for config 0 and 4 for some reason. Maybe the mapping between configs is still bugged

  /// calibration data for module gain in padiwa mapping for all configs
  /// This data is from the 15ns at thr 100 measurement
  /// This is THE CLOSEST TO PROPER DATA for now
  const std::vector<Float_t> evenL1ToT{0, 1.00516, 0.941679, 0.996602, 0.938697, 1.01379, 0.994034, 0.994879, 0.996319, 0.946602, 0.882979, 1.03918, 0.942259, 0.965832, 0.970083, 1.03512, 0.851115};
  const std::vector<Float_t> oddL1ToT {0, 0.94986, 1.00739, 0.988728, 0.940524, 0.97609, 0.976223, 0.969559, 1.04073, 1.00081, 1.00317, 1.16354, 0.938339, 1.01191, 0.966649, 1.07643, 1.01527};
  const std::vector<Float_t> oddL2ToT {0, 0.908184, 0.949028, 0.92109, 0.893274, 0.932635, 0.929634, 0.937754, 0.990187, 0.997177, 0.966501, 1.02128, 0.995851, 0.927551, 0.96848, 1.02195, 0.931882};
  const std::vector<Float_t> evenL2ToT{0, 0.945002, 0.951931, 1.01478, 0.977148, 1.05317, 0.979843, 0.94933, 1.02897, 1.07494, 1.00003, 1.12496, 1.06775, 1.00971, 1.05834, 1.06241, 1.06273};
  const std::vector<Float_t> evenL3ToT{0, 1.01542, 1.23263, 0.895245, 0.987732, 1.0101, 1.03212, 1.00866, 0.941641, 0.932012, 1.00862, 1.08677, 1.00308, 0.95071, 1.01135, 1.10522, 1.12427};
  const std::vector<Float_t> oddL3ToT {0, 1.10627, 1.08424, 1.07899, 1.03873, 1.06802, 0.483441, 1.07209, 1.09021, 1.05284, 0.97325, 1.09269, 0.993522, 0.926955, 0.948832, 1.1241, 1.04274};
  const std::vector<Float_t> oddL4ToT {0, 0.913229, 1.0021, 0.881695, 0.95427, 0.95506, 1.03781, 1.05169, 1.05813, 0.962686, 0.968182, 0.996458, 0.924963, 1.00067, 0.969411, 1.00694, 0.999656};
  const std::vector<Float_t> evenL4ToT{0, 1.06056, 1.01423, 0.989278, 1.00159, 1.0136, 0.982428, 0.987113, 0.995313, 0.968752, 1.03315, 1.06416, 0.896475, 0.879519, 1.03789, 1.06975, 0.973459};
  const std::vector<Float_t> evenL5ToT{0, 0.912107, 0.889598, 0.914224, 0.90943, 0.971451, 0.988836, 1.0443, 1.03953, 1.00065, 0.956255, 1.02575, 0.929022, 0.98261, 0.932769, 0.977462, 0.86774};
  const std::vector<Float_t> oddL5ToT {0, 0.98678, 1.00556, 1.06211, 1.00376, 1.05522, 1.01811, 1.04177, 1.04512, 1.09802, 0.994241, 1.08788, 0.984737, 1.01599, 1.05191, 1.06833, 1.02827};
  const std::vector<Float_t> oddL6ToT {0, 0.959972, 0.934731, 0.963809, 0.906432, 0.936054, 0.945976, 0.924591, 0.995694, 1.07502, 0.98929, 1.05085, 0.926802, 1.00291, 0.985536, 1.03092, 0.956101};
  const std::vector<Float_t> evenL6ToT{0, 1.03408, 0.965469, 1.01513, 0.945288, 1.03217, 0.978094, 1.02998, 1.0536, 1.0775, 0.966887, 1.11681, 0.947012, 1.02298, 1.00713, 1.01149, 0.9881};
  const std::vector<Float_t> evenL7ToT{0, 0.919742, 0.946011, 0.923928, 0.954334, 0.979316, 1.04481, 1.01167, 1.09111, 1.04175, 1.02009, 1.01209, 0.941175, 0.964628, 0.928821, 0.997715, 1.01764};
  const std::vector<Float_t> oddL7ToT {0, 1.09672, 1.09313, 1.05305, 1.09032, 1.0704, 1.04564, 1.04069, 1.08907, 1.11753, 1.03083, 1.12229, 1.04372, 1.01708, 1.06314, 1.12293, 1.09072};
  const std::vector<Float_t> oddL8ToT {0, 0.92899, 0.918904, 0.960584, 0.889824, 0.958196, 0.942646, 0.935016, 1.01289, 1.02634, 0.988673, 1.09515, 0.900691, 0.986994, 0.970973, 1.00001, 0.890438};
  const std::vector<Float_t> evenL8ToT{0, 1.04468, 1.00414, 1.03164, 1.00093, 1.02143, 0.982588, 1.05008, 1.06036, 1.04824, 0.944312, 1.07283, 0.904385, 0.946615, 0.991677, 0.971468, 0.987926};


  /// Module gain calibration from SiPM 100 threshold to signal height conversion and PION data
  const std::vector<Float_t> evenL1{0, 0.998188, 0.869224, 0.981296, 0.863594, 1.02523, 0.980131, 0.980386, 0.98999, 0.879023, 0.760695, 1.08153, 0.869402, 0.918262, 0.919989, 1.0778, 0.705153};
  const std::vector<Float_t> oddL1 {0, 0.895516, 1.00801, 0.964643, 0.857936, 0.94458, 0.940309, 0.923468, 1.09292, 0.995095, 1.00984, 1.4301, 0.870226, 1.02111, 0.921426, 1.17261, 1.00957};
  const std::vector<Float_t> oddL2 {0, 0.809752, 0.883818, 0.83191, 0.782905, 0.85091, 0.847118, 0.864648, 0.975181, 0.989496, 0.923652, 1.04311, 0.985776, 0.845518, 0.925292, 1.0482, 0.837781};
  const std::vector<Float_t> evenL2{0, 0.863693, 0.872222, 1.00692, 0.926993, 1.09415, 0.932937, 0.869411, 1.04037, 1.15595, 0.979709, 1.29162, 1.13955, 0.994946, 1.11991, 1.12501, 1.13282};
  const std::vector<Float_t> evenL3{0, 1.02716, 1.61374, 0.771743, 0.970502, 1.02043, 1.07542, 1.01421, 0.862025, 0.832847, 1.0129, 1.20542, 0.990248, 0.891995, 1.01526, 1.2539, 1.33082};
  const std::vector<Float_t> oddL3 {0, 1.24085, 1.21127, 1.18372, 1.06463, 1.14573, 0.296313, 1.15941, 1.19648, 1.13061, 0.930366, 1.20882, 0.952404, 0.825186, 0.852413, 1.28198, 1.0838};
  const std::vector<Float_t> oddL4 {0, 0.819272, 0.994891, 0.755212, 0.891102, 0.897473, 1.0751, 1.11478, 1.13422, 0.907579, 0.91746, 0.963075, 0.843607, 0.994816, 0.931135, 1.00651, 0.963237};
  const std::vector<Float_t> evenL4{0, 1.13229, 1.02508, 0.95809, 0.986931, 1.00947, 0.952782, 0.962101, 0.979555, 0.914618, 1.06292, 1.13455, 0.778935, 0.74843, 1.06957, 1.14529, 0.925121};
  const std::vector<Float_t> evenL5{0, 0.827386, 0.786722, 0.822749, 0.803012, 0.927521, 0.97208, 1.09706, 1.07904, 1.0068, 0.902957, 1.03756, 0.848911, 0.956262, 0.845193, 0.939398, 0.719372};
  const std::vector<Float_t> oddL5 {0, 0.964004, 1.00141, 1.13572, 0.999925, 1.1272, 1.02891, 1.09117, 1.09856, 1.24633, 0.9787, 1.23172, 0.974946, 1.03566, 1.10818, 1.16299, 1.06365};
  const std::vector<Float_t> oddL6 {0, 0.911313, 0.85578, 0.915081, 0.807932, 0.859762, 0.892945, 0.834416, 0.980248, 1.17866, 0.967518, 1.10893, 0.838986, 1.01411, 0.963303, 1.06771, 0.89489};
  const std::vector<Float_t> evenL6{0, 1.05543, 0.910633, 1.01851, 0.869368, 1.04841, 0.933001, 1.04331, 1.11066, 1.15502, 0.908938, 1.26181, 0.869303, 1.03152, 0.988863, 1.00932, 0.964436};
  const std::vector<Float_t> evenL7{0, 0.841308, 0.881812, 0.837628, 0.885453, 0.944969, 1.09703, 1.01679, 1.21673, 1.0946, 1.03851, 1.00161, 0.872289, 0.916652, 0.847089, 0.98993, 1.03716};
  const std::vector<Float_t> oddL7 {0, 1.24535, 1.22254, 1.12892, 1.20678, 1.16598, 1.10338, 1.10148, 1.20538, 1.32116, 1.06459, 1.33315, 1.11452, 1.05188, 1.14194, 1.33002, 1.24006};
  const std::vector<Float_t> oddL8 {0, 0.853413, 0.828637, 0.914405, 0.801596, 0.919852, 0.903553, 0.85678, 1.03658, 1.05049, 0.980588, 1.23755, 0.799858, 1.00374, 0.929051, 0.99872, 0.791233};
  const std::vector<Float_t> evenL8{0, 1.092, 1.01867, 1.05867, 0.994969, 1.03617, 0.961725, 1.11118, 1.15954, 1.10362, 0.877745, 1.21505, 0.826899, 0.893722, 0.975042, 0.924554, 0.956497};

  /// Module gain calibration from SiPM 100 threshold to signal height conversion and PROTON data
  const std::vector<Float_t> evenL1Protons{0, 1.03037, 0.848169, 1.03656, 0.879608, 1.1897, 1.01832, 1.00699, 1.06184, 0.971215, 0.841209, 1.21554, 0.937615, 1.03589, 0.956818, 1.20796, 0.762198};
  const std::vector<Float_t> oddL1Protons {0, 0.820399, 0.916145, 0.939042, 0.753381, 0.943445, 0.877547, 0.982739, 1.04514, 1.12537, 0.955044, 1.52993, 0.936309, 1.04332, 0.924127, 1.18169, 1.02636};
  const std::vector<Float_t> oddL2Protons {0, 0.818971, 0.939571, 0.99185, 0.798597, 0.997658, 0.951966, 0.917336, 1.03995, 1.20414, 1.07488, 1.23165, 0.998569, 0.966853, 0.89391, 1.27049, 0.903608};
  const std::vector<Float_t> evenL2Protons{0, 0.747359, 0.716802, 0.880372, 0.851991, 1.1382, 0.818829, 0.9199, 1.00411, 1.16663, 0.853257, 1.44945, 1.0305, 0.98411, 0.992185, 1.18911, 1.25719};
  const std::vector<Float_t> evenL3Protons{0, 0.959844, 0, 0.731838, 0.876855, 1.02911, 0.899825, 0.883842, 0.90382, 0.945035, 1.06511, 1.24391, 0.953154, 0.828103, 0.936579, 1.40059, 1.34238};
  const std::vector<Float_t> oddL3Protons {0, 1.1489, 1.03188, 0.917096, 0.860162, 0.955199, 0, 1.00764, 1.09909, 1.08956, 0.814526, 1.30426, 0.845568, 0.777093, 0.756196, 1.27405, 1.11879};
  const std::vector<Float_t> oddL4Protons {0, 0.806365, 0.8637, 0.753966, 0.818876, 0.991923, 1.12059, 1.14856, 1.14417, 0.926385, 1.00258, 1.35156, 0.813196, 1.01006, 0.943165, 1.18678, 1.11812};
  const std::vector<Float_t> evenL4Protons{0, 1.01568, 0.968274, 0.915342, 0.884504, 1.00582, 0.879881, 0.994435, 0.989744, 1.00444, 1.01638, 1.35737, 0.856564, 0.733685, 1.04009, 1.19988, 1.13791};
  const std::vector<Float_t> evenL5Protons{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> oddL5Protons {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> oddL6Protons {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> evenL6Protons{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> evenL7Protons{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> oddL7Protons {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> oddL8Protons {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> evenL8Protons{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  const std::vector<Float_t> evenL1ProtonsToT{0, 1.01057, 0.953287, 1.01332, 0.963765, 1.05319, 1.00536, 1.00369, 1.01998, 0.994007, 0.954093, 1.06204, 0.983873, 1.01299, 0.990267, 1.05762, 0.921947};
  const std::vector<Float_t> oddL1ProtonsToT {0, 0.947272, 0.975039, 0.984596, 0.919836, 0.986507, 0.964551, 0.999916, 1.01587, 1.03925, 0.989569, 1.12761, 0.983562, 1.01648, 0.982304, 1.05264, 1.01499};
  const std::vector<Float_t> oddL2ProtonsToT {0, 0.944598, 0.983725, 0.998393, 0.932759, 1.00009, 0.987515, 0.974722, 1.0142, 1.05672, 1.02421, 1.06627, 0.998378, 0.993601, 0.971039, 1.07449, 0.979308};
  const std::vector<Float_t> evenL2ProtonsToT{0, 0.925113, 0.91317, 0.969782, 0.960476, 1.0428, 0.949313, 0.984762, 1.00696, 1.04619, 0.958971, 1.10722, 1.00955, 1.0003, 1.00288, 1.05199, 1.07051};
  const std::vector<Float_t> evenL3ProtonsToT{0, 0.995827, 0, 0.917031, 0.966844, 1.01205, 0.973729, 0.968431, 0.976107, 0.989624, 1.02213, 1.0652, 0.989929, 0.951786, 0.985082, 1.09828, 1.08795};
  const std::vector<Float_t> oddL3ProtonsToT {0, 1.0413, 1.01204, 0.978311, 0.96057, 0.991689, 0, 1.00838, 1.03157, 1.02856, 0.946844, 1.07601, 0.956634, 0.934894, 0.927116, 1.07189, 1.03419};
  const std::vector<Float_t> oddL4ProtonsToT {0, 0.941198, 0.960171, 0.925316, 0.943235, 0.999258, 1.03473, 1.04624, 1.0413, 0.979952, 1.01062, 1.08586, 0.946007, 1.00534, 0.987022, 1.05462, 1.03914};
  const std::vector<Float_t> evenL4ProtonsToT{0, 1.01142, 0.994083, 0.97588, 0.969063, 1.00397, 0.967519, 1.00196, 1.0009, 1.00317, 1.00607, 1.086, 0.958494, 0.915946, 1.01221, 1.0528, 1.04052};
  const std::vector<Float_t> evenL5ProtonsToT{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> oddL5ProtonsToT {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> oddL6ProtonsToT {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> evenL6ProtonsToT{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> evenL7ProtonsToT{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> oddL7ProtonsToT {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> oddL8ProtonsToT {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const std::vector<Float_t> evenL8ProtonsToT{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  /// The E18 gain map
  const std::vector<Float_t> evenL1E18{0, 0.969561, 0.864863, 0.946059, 0.867142, 1.19787, 1.05303, 0.893182, 0.949806, 0.828129, 0.942268, 0.92256, 0.901908, 1.05569, 0.925499, 1.05727, 0.827669};
  const std::vector<Float_t> oddL1E18 {0, 0.861221, 0.965512, 0.791897, 0.885203, 0.877823, 0.867295, 0.829045, 1.05956, 0.999784, 1.14925, 1.12559, 1.10732, 1.21092, 0.910463, 1.09132, 1.19287};
  const std::vector<Float_t> oddL2E18 {0, 0.786471, 1.05938, 1.05457, 0.861465, 1.01837, 0.76531, 0.797922, 0.902716, 0.868343, 0.956923, 0.793725, 0.924423, 0.885344, 0.776616, 0.857755, 0.827788};
  const std::vector<Float_t> evenL2E18{0, 0.90954, 0.959668, 1.10228, 0.88164, 1.10299, 0.957894, 1.04688, 1.08706, 0.975571, 0.886204, 1.00989, 0.868917, 0.8319, 0.846531, 0.827625, 1.04245};
  const std::vector<Float_t> evenL3E18{0, 0.966929, 0.999962, 0.731866, 0.944429, 1.09669, 0.909569, 1.07625, 0.800968, 0.788387, 1.17599, 1.10646, 0.954296, 1.03841, 1.11544, 1.38842, 1.15908};
  const std::vector<Float_t> oddL3E18 {0, 1.08628, 0.954414, 1.01229, 0.861019, 1.01666, 0, 1.22479, 1.13428, 1.03984, 0.897011, 0.913572, 0.91199, 1.05075, 0.862226, 1.16295, 1.06303};
  const std::vector<Float_t> oddL4E18 {0, 0.931672, 1.04098, 0.658629, 0.886751, 0.836383, 1.07507, 1.2089, 0.95905, 0.794396, 0.969706, 0.83019, 0.763332, 1.10319, 0.935831, 0.885376, 1.15949};
  const std::vector<Float_t> evenL4E18{0, 1.17157, 1.02472, 0.993623, 1.14023, 0.971641, 1.03348, 1.00722, 0.945378, 0.886796, 1.16471, 0.958752, 0.859107, 0.810029, 0.988501, 0.97971, 0.894559};
  const std::vector<Float_t> evenL5E18{0, 1.08079, 0.939735, 0.873558, 0.859743, 0.940193, 1.06118, 1.06368, 1.02937, 1.12714, 1.09771, 1.09249, 0.988776, 1.17771, 1.02888, 0.72756, 0.905587};
  const std::vector<Float_t> oddL5E18 {0, 1.12072, 1.05495, 0.879246, 1.23416, 1.20518, 1.02017, 0.943164, 1.09648, 1.13979, 0.942875, 1.02371, 1.04916, 1.07833, 1.03301, 1.25201, 1.27666};
  const std::vector<Float_t> oddL6E18 {0, 0.987265, 0.93903, 1.07204, 0.725616, 0.920545, 0.754098, 0.823088, 0.951362, 1.23097, 0.915464, 0.957061, 0.836357, 0.874567, 1.07375, 0.922043, 0.847697};
  const std::vector<Float_t> evenL6E18{0, 1.19048, 1.07449, 1.12796, 1.07515, 1.07173, 0.926418, 1.03114, 0.990132, 1.03043, 0.849754, 0.835833, 0.806346, 0.791853, 0.826169, 0.932687, 0.961961, };
  const std::vector<Float_t> evenL7E18{0, 0.67509, 0.730417, 0.764924, 0.814831, 1.00812, 0.92892, 1.05945, 1.00411, 0.998093, 1.08343, 1.00116, 0.951893, 0.946911, 1.04002, 1.04902, 0.857053};
  const std::vector<Float_t> oddL7E18 {0, 0.929933, 1.02151, 1.12066, 1.03029, 1.11577, 1.14389, 1.17854, 1.06252, 1.41089, 1.21955, 1.01592, 1.17853, 1.48629, 1.41459, 1.4157, 1.5835};
  const std::vector<Float_t> oddL8E18 {0, 1.04829, 0.940543, 1.03382, 1.13111, 1.21263, 1.05124, 0.879264, 0.982071, 0.945926, 1.06941, 0.972253, 1.01753, 1.21697, 1.0195, 1.0183, 1.02523};
  const std::vector<Float_t> evenL8E18{0, 1.25611, 1.1119, 1.01072, 1.15287, 1.12274, 1.15456, 1.27304, 1.36448, 1.24726, 1.01053, 1.12303, 0.935794, 1.03196, 1.07967, 0.872062, 0.835493};

  const std::map<PadiwaSocket, std::vector<Float_t>> testModuleGainMapping {
    { PadiwaSocket::L1even, evenL1 },
    { PadiwaSocket::L2even, evenL2 },
    { PadiwaSocket::L3even, evenL3 },
    { PadiwaSocket::L4even, evenL4 },
    { PadiwaSocket::L5even, evenL5 },
    { PadiwaSocket::L6even, evenL6 },
    { PadiwaSocket::L7even, evenL7 },
    { PadiwaSocket::L8even, evenL8 },
    { PadiwaSocket::L1odd,  oddL1  },
    { PadiwaSocket::L2odd,  oddL2  },
    { PadiwaSocket::L3odd,  oddL3  },
    { PadiwaSocket::L4odd,  oddL4  },
    { PadiwaSocket::L5odd,  oddL5  },
    { PadiwaSocket::L6odd,  oddL6  },
    { PadiwaSocket::L7odd,  oddL7  },
    { PadiwaSocket::L8odd,  oddL8  }
  };

  const std::map<PadiwaSocket, std::vector<Float_t>> testModuleGainMappingToT {
    { PadiwaSocket::L1even, evenL1ToT },
    { PadiwaSocket::L2even, evenL2ToT },
    { PadiwaSocket::L3even, evenL3ToT },
    { PadiwaSocket::L4even, evenL4ToT },
    { PadiwaSocket::L5even, evenL5ToT },
    { PadiwaSocket::L6even, evenL6ToT },
    { PadiwaSocket::L7even, evenL7ToT },
    { PadiwaSocket::L8even, evenL8ToT },
    { PadiwaSocket::L1odd,  oddL1ToT  },
    { PadiwaSocket::L2odd,  oddL2ToT  },
    { PadiwaSocket::L3odd,  oddL3ToT  },
    { PadiwaSocket::L4odd,  oddL4ToT  },
    { PadiwaSocket::L5odd,  oddL5ToT  },
    { PadiwaSocket::L6odd,  oddL6ToT  },
    { PadiwaSocket::L7odd,  oddL7ToT  },
    { PadiwaSocket::L8odd,  oddL8ToT  }
  };

  const std::map<PadiwaSocket, std::vector<Float_t>> testModuleGainMappingProtons {
    { PadiwaSocket::L1even, evenL1Protons },
    { PadiwaSocket::L2even, evenL2Protons },
    { PadiwaSocket::L3even, evenL3Protons },
    { PadiwaSocket::L4even, evenL4Protons },
    { PadiwaSocket::L5even, evenL5Protons },
    { PadiwaSocket::L6even, evenL6Protons },
    { PadiwaSocket::L7even, evenL7Protons },
    { PadiwaSocket::L8even, evenL8Protons },
    { PadiwaSocket::L1odd,  oddL1Protons  },
    { PadiwaSocket::L2odd,  oddL2Protons  },
    { PadiwaSocket::L3odd,  oddL3Protons  },
    { PadiwaSocket::L4odd,  oddL4Protons  },
    { PadiwaSocket::L5odd,  oddL5Protons  },
    { PadiwaSocket::L6odd,  oddL6Protons  },
    { PadiwaSocket::L7odd,  oddL7Protons  },
    { PadiwaSocket::L8odd,  oddL8Protons  }
  };

  const std::map<PadiwaSocket, std::vector<Float_t>> testModuleGainMappingProtonsToT {
    { PadiwaSocket::L1even, evenL1ProtonsToT },
    { PadiwaSocket::L2even, evenL2ProtonsToT },
    { PadiwaSocket::L3even, evenL3ProtonsToT },
    { PadiwaSocket::L4even, evenL4ProtonsToT },
    { PadiwaSocket::L5even, evenL5ProtonsToT },
    { PadiwaSocket::L6even, evenL6ProtonsToT },
    { PadiwaSocket::L7even, evenL7ProtonsToT },
    { PadiwaSocket::L8even, evenL8ProtonsToT },
    { PadiwaSocket::L1odd,  oddL1ProtonsToT  },
    { PadiwaSocket::L2odd,  oddL2ProtonsToT  },
    { PadiwaSocket::L3odd,  oddL3ProtonsToT  },
    { PadiwaSocket::L4odd,  oddL4ProtonsToT  },
    { PadiwaSocket::L5odd,  oddL5ProtonsToT  },
    { PadiwaSocket::L6odd,  oddL6ProtonsToT  },
    { PadiwaSocket::L7odd,  oddL7ProtonsToT  },
    { PadiwaSocket::L8odd,  oddL8ProtonsToT  }
  };

  const std::map<PadiwaSocket, std::vector<Float_t>> testModuleGainMappingE18 {
    { PadiwaSocket::L1even, evenL1E18 },
    { PadiwaSocket::L2even, evenL2E18 },
    { PadiwaSocket::L3even, evenL3E18 },
    { PadiwaSocket::L4even, evenL4E18 },
    { PadiwaSocket::L5even, evenL5E18 },
    { PadiwaSocket::L6even, evenL6E18 },
    { PadiwaSocket::L7even, evenL7E18 },
    { PadiwaSocket::L8even, evenL8E18 },
    { PadiwaSocket::L1odd,  oddL1E18  },
    { PadiwaSocket::L2odd,  oddL2E18  },
    { PadiwaSocket::L3odd,  oddL3E18  },
    { PadiwaSocket::L4odd,  oddL4E18  },
    { PadiwaSocket::L5odd,  oddL5E18  },
    { PadiwaSocket::L6odd,  oddL6E18  },
    { PadiwaSocket::L7odd,  oddL7E18  },
    { PadiwaSocket::L8odd,  oddL8E18  }
  };

  const std::map<std::vector<Float_t>, std::vector<Float_t>> gainMapCompareHelper {
    { evenL1, evenL1E18 },
    { evenL2, evenL2E18 },
    { evenL3, evenL3E18 },
    { evenL4, evenL4E18 },
    { evenL5, evenL5E18 },
    { evenL6, evenL6E18 },
    { evenL7, evenL7E18 },
    { evenL8, evenL8E18 },
    { oddL1, oddL1E18 },
    { oddL2, oddL2E18 },
    { oddL3, oddL3E18 },
    { oddL4, oddL4E18 },
    { oddL5, oddL5E18 },
    { oddL6, oddL6E18 },
    { oddL7, oddL7E18 },
    { oddL8, oddL8E18 }
  };

  //===========================================================================================================================

  /// T0 calibration is taken from lab measurement
  const std::map<Padiwa, std::vector<Float_t>> padiwaTimeCorr{
    { Padiwa::p1500_0, {0, 6.93889E-16, 1.11542,  0.8119,      2.251916, -0.922404, -0.453114,  0.8883776, 0.155795, -0.938464, -1.540544,   -0.809134, -1.552644, -0.692764, -0.464704,  -2.004604, -2.406544} },
    { Padiwa::p1500_1, {0, 2.517826,    2.66 ,    3.397055,    4.096766,  2.263641,  1.699249,  0.276526,  1.177086, -1.945154, -0.044184,   -1.795594, -1.466974,  5.072366,  5.563236,   3.990506,  1.566279} },
    { Padiwa::p1510_0, {0, 6.93889E-16, 1.11542,  1.24223,     2.67931,  -0.456091, -0.0149989, 1.3712,    0.504082, -0.370934, -1.08655,    -0.454727, -1.31195,  -0.195293, -0.0461142, -1.59216,  -1.91812 } },
    { Padiwa::p1510_1, {0, 2.891333,    2.330744, 2.8694499,   4.381363,  2.458993,  2.126462,  0.312753,  1.072663, -2.297937, -0.179747,   -1.741817, -1.241587,  5.008823,  5.596693,   4.036363,  1.282623} },
    { Padiwa::p1520_0, {0, 6.93889E-16, 1.11542,  0.934946,    2.728777, -0.540214, -0.227886,  1.211543,  0.485613, -0.730095, -1.434413,   -0.420441, -1.241343, -0.531263, -0.35043,   -1.803833, -2.009693} },
    { Padiwa::p1520_1, {0, 2.891333,    2.0,      2.890576515, 3.845229,  1.818433,  1.217863, -0.1303969, 0.809853, -2.326017, -0.38771699, -2.312257, -1.806617,  4.719783,  5.478213,   3.806037,  1.246643} },
    { Padiwa::p1530_0, {0, 6.93889E-16, 1.32542,  1.16687,     2.82989,  -0.452611, -0.0387911, 1.36946,   0.700189, -0.505421, -0.957971,   -0.212678, -1.27323,  -0.0845989, 0.0808505, -1.39277,  -1.85497 } },
    { Padiwa::p1530_1, {0, 2.517826,    3.275819, 4.234145,    4.983406,  3.082846,  2.491707,  1.043966,  2.101356, -1.143034,  0.760086,   -0.998524, -0.563224,  5.807756,  6.502226,   4.881126,  2.503614} }
  };

  const std::map<Padiwa, std::string> padiwaNameMap {
    { Padiwa::p1500_0, "1500_0" },
    { Padiwa::p1500_1, "1500_1" },
    { Padiwa::p1510_0, "1510_0" },
    { Padiwa::p1510_1, "1510_1" },
    { Padiwa::p1520_0, "1520_0" },
    { Padiwa::p1520_1, "1520_1" },
    { Padiwa::p1530_0, "1530_0" },
    { Padiwa::p1530_1, "1530_1" }
  };

  const std::map<Padiwa, Int_t> padiwaPosMap {
    { Padiwa::p1500_0, 0 },
    { Padiwa::p1500_1, 1 },
    { Padiwa::p1510_0, 2 },
    { Padiwa::p1510_1, 3 },
    { Padiwa::p1520_0, 4 },
    { Padiwa::p1520_1, 5 },
    { Padiwa::p1530_0, 6 },
    { Padiwa::p1530_1, 7 }
  };

  const std::vector<std::string> padiwaNames{ "1500_0",
                                              "1500_1",
                                              "1510_0",
                                              "1510_1",
                                              "1520_0",
                                              "1520_1",
                                              "1530_0",
                                              "1530_1"
  };

} // namespace constants

#endif