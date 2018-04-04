// Filename: VCINSStaggeredConservativePPMConvectiveOperator.cpp
// Created on 01 April 2018 by Nishant Nangia
//
// Copyright (c) 2002-2017, Nishant Nangia
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of The University of North Carolina nor the names of
//      its contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/////////////////////////////// INCLUDES /////////////////////////////////////

#include <stddef.h>
#include <ostream>
#include <string>
#include <vector>

#include "Box.h"
#include "CartesianPatchGeometry.h"
#include "FaceData.h"
#include "HierarchyDataOpsManager.h"
#include "HierarchySideDataOpsReal.h"
#include "IBAMR_config.h"
#include "Index.h"
#include "IntVector.h"
#include "MultiblockDataTranslator.h"
#include "Patch.h"
#include "PatchHierarchy.h"
#include "PatchLevel.h"
#include "SAMRAIVectorReal.h"
#include "SideData.h"
#include "SideGeometry.h"
#include "SideVariable.h"
#include "Variable.h"
#include "VariableContext.h"
#include "VariableDatabase.h"
#include "boost/array.hpp"
#include "ibamr/ConvectiveOperator.h"
#include "ibamr/VCINSStaggeredConservativePPMConvectiveOperator.h"
#include "ibamr/StaggeredStokesPhysicalBoundaryHelper.h"
#include "ibamr/ibamr_enums.h"
#include "ibamr/ibamr_utilities.h"
#include "ibamr/namespaces.h" // IWYU pragma: keep
#include "ibtk/HierarchyGhostCellInterpolation.h"
#include "tbox/Database.h"
#include "tbox/Pointer.h"
#include "tbox/Timer.h"
#include "tbox/TimerManager.h"
#include "tbox/Utilities.h"

namespace SAMRAI
{
namespace solv
{
template <int DIM>
class RobinBcCoefStrategy;
} // namespace solv
} // namespace SAMRAI

// FORTRAN ROUTINES
#if (NDIM == 2)
#define CONVECT_DERIVATIVE_FC IBAMR_FC_FUNC_(convect_derivative2d, CONVECT_DERIVATIVE2D)
#define VC_UPDATE_DENSITY_FC IBAMR_FC_FUNC_(vc_update_density2d, VC_UPDATE_DENSITY2D)
#define GODUNOV_EXTRAPOLATE_FC IBAMR_FC_FUNC_(godunov_extrapolate2d, GODUNOV_EXTRAPOLATE2D)
#define NAVIER_STOKES_INTERP_COMPS_FC IBAMR_FC_FUNC_(navier_stokes_interp_comps2d, NAVIER_STOKES_INTERP_COMPS2D)
#define VC_NAVIER_STOKES_UPWIND_DENSITY_FC IBAMR_FC_FUNC_(vc_navier_stokes_upwind_density2d, VC_NAVIER_STOKES_UPWIND_DENSITY2D)
#define VC_NAVIER_STOKES_RESET_ADV_MOMENTUM_FC                                                                            \
    IBAMR_FC_FUNC_(vc_navier_stokes_reset_adv_momentum2d, VC_NAVIER_STOKES_RESET_ADV_MOMENTUM2D)
#define SKEW_SYM_DERIVATIVE_FC IBAMR_FC_FUNC_(skew_sym_derivative2d, SKEW_SYM_DERIVATIVE2D)
#endif

#if (NDIM == 3)
#define CONVECT_DERIVATIVE_FC IBAMR_FC_FUNC_(convect_derivative3d, CONVECT_DERIVATIVE3D)
#define VC_UPDATE_DENSITY_FC IBAMR_FC_FUNC_(vc_update_density3d, VC_UPDATE_DENSITY3D)
#define GODUNOV_EXTRAPOLATE_FC IBAMR_FC_FUNC_(godunov_extrapolate3d, GODUNOV_EXTRAPOLATE3D)
#define NAVIER_STOKES_INTERP_COMPS_FC IBAMR_FC_FUNC_(navier_stokes_interp_comps3d, NAVIER_STOKES_INTERP_COMPS3D)
#define VC_NAVIER_STOKES_UPWIND_DENSITY_FC IBAMR_FC_FUNC_(vc_navier_stokes_upwind_density3d, VC_NAVIER_STOKES_UPWIND_DENSITY3D)
#define VC_NAVIER_STOKES_RESET_ADV_MOMENTUM_FC                                                                            \
    IBAMR_FC_FUNC_(vc_navier_stokes_reset_adv_momentum3d, VC_NAVIER_STOKES_RESET_ADV_MOMENTUM3D)
#define SKEW_SYM_DERIVATIVE_FC IBAMR_FC_FUNC_(skew_sym_derivative3d, SKEW_SYM_DERIVATIVE3D)
#endif

extern "C" {
void CONVECT_DERIVATIVE_FC(const double*,
#if (NDIM == 2)
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const double*,
                           const double*,
                           const double*,
                           const double*,
                           const int&,
                           const int&,
#endif
#if (NDIM == 3)
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const int&,
                           const double*,
                           const double*,
                           const double*,
                           const double*,
                           const double*,
                           const double*,
                           const int&,
                           const int&,
                           const int&,
#endif
                           double*);
void VC_UPDATE_DENSITY_FC(const double*,
                          const double&,
#if (NDIM == 2)
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const double*,
                          const double*,
                          const int&,
                          const int&,
                          const double*,
                          const int&,
                          const int&,
#endif
#if (NDIM == 3)
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const int&,
                          const double*,
                          const double*,
                          const double*,
                          const int&,
                          const int&,
                          const int&,
                          const double*,
                          const int&,
                          const int&,
                          const int&,
#endif
                          double*);

void GODUNOV_EXTRAPOLATE_FC(
#if (NDIM == 2)
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    double*,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    double*,
    double*
#endif
#if (NDIM == 3)
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    double*,
    double*,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    double*,
    double*,
    double*
#endif
    );

void NAVIER_STOKES_INTERP_COMPS_FC(
#if (NDIM == 2)
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*
#endif
#if (NDIM == 3)
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*
#endif
    );

void VC_NAVIER_STOKES_UPWIND_DENSITY_FC(
#if (NDIM == 2)
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const int&,
    const int&,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const int&,
    const int&,
    double*,
    double*
#endif
#if (NDIM == 3)
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*
#endif
    );

void VC_NAVIER_STOKES_RESET_ADV_MOMENTUM_FC(
#if (NDIM == 2)
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    const int&,
    const int&,
    const double*,
    const double*,
    const int&,
    const int&,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    const int&,
    const int&,
    const double*,
    const double*,
    const int&,
    const int&,
    const double*,
    const double*
#endif
#if (NDIM == 3)
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    const int&,
    double*,
    double*,
    double*,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*,
    const int&,
    const int&,
    const int&,
    const double*,
    const double*,
    const double*
#endif
    );

void SKEW_SYM_DERIVATIVE_FC(const double*,
#if (NDIM == 2)
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const double*,
                            const double*,
                            const double*,
                            const double*,
                            const int&,
                            const int&,
#endif
#if (NDIM == 3)
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const int&,
                            const double*,
                            const double*,
                            const double*,
                            const double*,
                            const double*,
                            const double*,
                            const int&,
                            const int&,
                            const int&,
#endif
                            double*);
}

/////////////////////////////// NAMESPACE ////////////////////////////////////

namespace IBAMR
{
/////////////////////////////// STATIC ///////////////////////////////////////

namespace
{
// NOTE: The number of ghost cells required by the Godunov advection scheme
// depends on the order of the reconstruction.  These values were chosen to work
// with xsPPM7 (the modified piecewise parabolic method of Rider, Greenough, and
// Kamm).
static const int GADVECTG = 4;
static const int GRHOINTERPG = 2;
static const int NOGHOSTS = 0;

// Timers.
static Timer* t_apply_convective_operator;
static Timer* t_apply;
static Timer* t_initialize_operator_state;
static Timer* t_deallocate_operator_state;
}

/////////////////////////////// PUBLIC ///////////////////////////////////////

VCINSStaggeredConservativePPMConvectiveOperator::VCINSStaggeredConservativePPMConvectiveOperator(
    const std::string& object_name,
    Pointer<Database> input_db,
    const ConvectiveDifferencingType difference_form,
    const std::vector<RobinBcCoefStrategy<NDIM>*>& bc_coefs)
    : ConvectiveOperator(object_name, difference_form),
      d_bc_coefs(bc_coefs),
      d_bdry_extrap_type("CONSTANT"),
      d_hierarchy(NULL),
      d_coarsest_ln(-1),
      d_finest_ln(-1),
      d_rho_is_set(false),
      d_dt_is_set(false),
      d_dt(-1.0),
      d_U_var(NULL),
      d_U_scratch_idx(-1),
      d_rho_interp_var(NULL),
      d_rho_interp_current_idx(-1),
      d_rho_interp_scratch_idx(-1),
      d_rho_interp_new_idx(-1)
{
    if ( d_difference_form != CONSERVATIVE)
    {
        TBOX_ERROR("VCINSStaggeredConservativePPMConvectiveOperator::VCINSStaggeredConservativePPMConvectiveOperator():\n"
                   << "  unsupported differencing form: "
                   << enum_to_string<ConvectiveDifferencingType>(d_difference_form)
                   << " \n"
                   << "  valid choices are: CONSERVATIVE\n");
    }

    if (input_db)
    {
        if (input_db->keyExists("bdry_extrap_type")) d_bdry_extrap_type = input_db->getString("bdry_extrap_type");
    }

    VariableDatabase<NDIM>* var_db = VariableDatabase<NDIM>::getDatabase();
    Pointer<VariableContext> context = var_db->getContext("VCINSStaggeredConservativePPMConvectiveOperator::CONTEXT");

    const std::string U_var_name = "VCINSStaggeredConservativePPMConvectiveOperator::U";
    d_U_var = var_db->getVariable(U_var_name);
    if (d_U_var)
    {
        d_U_scratch_idx = var_db->mapVariableAndContextToIndex(d_U_var, context);
    }
    else
    {
        d_U_var = new SideVariable<NDIM, double>(U_var_name);
        d_U_scratch_idx = var_db->registerVariableAndContext(d_U_var, context, IntVector<NDIM>(GADVECTG));
    }

#if !defined(NDEBUG)
    TBOX_ASSERT(d_U_scratch_idx >= 0);
#endif

    const std::string rho_interp_name = "VCINSStaggeredConservativePPMConvectiveOperator::RHO_INTERP";
    d_rho_interp_var = var_db->getVariable(rho_interp_name);
    if (d_rho_interp_var)
    {
        d_rho_interp_scratch_idx = var_db->mapVariableAndContextToIndex(d_rho_interp_var, var_db->getContext(rho_interp_name + "::SCRATCH"));
        d_rho_interp_new_idx = var_db->mapVariableAndContextToIndex(d_rho_interp_var, var_db->getContext(rho_interp_name + "::NEW"));
    }
    else
    {
        d_rho_interp_var = new SideVariable<NDIM, double>(rho_interp_name);
        d_rho_interp_scratch_idx = var_db->registerVariableAndContext(d_rho_interp_var, var_db->getContext(rho_interp_name + "::SCRATCH"), IntVector<NDIM>(GRHOINTERPG));
        d_rho_interp_new_idx = var_db->registerVariableAndContext(d_rho_interp_var, var_db->getContext(rho_interp_name + "::NEW"), IntVector<NDIM>(NOGHOSTS));
    }
#if !defined(NDEBUG)
    TBOX_ASSERT(d_rho_interp_scratch_idx >= 0);
    TBOX_ASSERT(d_rho_interp_new_idx >= 0);
#endif

    // Setup Timers.
    IBAMR_DO_ONCE(t_apply_convective_operator = TimerManager::getManager()->getTimer(
                      "IBAMR::VCINSStaggeredConservativePPMConvectiveOperator::applyConvectiveOperator()");
                  t_apply = TimerManager::getManager()->getTimer("IBAMR::VCINSStaggeredConservativePPMConvectiveOperator::apply()");
                  t_initialize_operator_state = TimerManager::getManager()->getTimer(
                      "IBAMR::VCINSStaggeredConservativePPMConvectiveOperator::initializeOperatorState()");
                  t_deallocate_operator_state = TimerManager::getManager()->getTimer(
                      "IBAMR::VCINSStaggeredConservativePPMConvectiveOperator::deallocateOperatorState()"););
    return;
} // VCINSStaggeredConservativePPMConvectiveOperator

VCINSStaggeredConservativePPMConvectiveOperator::~VCINSStaggeredConservativePPMConvectiveOperator()
{
    deallocateOperatorState();
    return;
} // ~VCINSStaggeredConservativePPMConvectiveOperator

void
VCINSStaggeredConservativePPMConvectiveOperator::applyConvectiveOperator(const int U_idx, const int N_idx)
{
    // Debugging
    HierarchyDataOpsManager<NDIM>* hier_ops_manager = HierarchyDataOpsManager<NDIM>::getManager();
    d_hier_sc_data_ops = hier_ops_manager->getOperationsDouble(new SideVariable<NDIM, double>("sc_var"), d_hierarchy, true);


    IBAMR_TIMER_START(t_apply_convective_operator);
#if !defined(NDEBUG)
    if (!d_is_initialized)
    {
        TBOX_ERROR("VCINSStaggeredConservativePPMConvectiveOperator::applyConvectiveOperator():\n"
                   << "  operator must be initialized prior to call to applyConvectiveOperator\n");
    }
    TBOX_ASSERT(U_idx == d_u_idx);

    if (!d_rho_is_set)
    {
        TBOX_ERROR("VCINSStaggeredConservativePPMConvectiveOperator::applyConvectiveOperator():\n"
                   << "  a side-centered density field must be set via setInterpolatedDensityPatchDataIndex()\n"
                   << "  prior to call to applyConvectiveOperator\n");
    }
    TBOX_ASSERT(d_rho_interp_current_idx >= 0);

    if (!d_dt_is_set)
    {
        TBOX_ERROR("VCINSStaggeredConservativePPMConvectiveOperator::applyConvectiveOperator():\n"
                   << "  the current time step size must be set via  setTimeStepSize()\n"
                   << "  prior to call to applyConvectiveOperator\n");
    }
    TBOX_ASSERT(d_dt >= 0.0);
#endif

    // Allocate scratch data.
    for (int ln = d_coarsest_ln; ln <= d_finest_ln; ++ln)
    {
        Pointer<PatchLevel<NDIM> > level = d_hierarchy->getPatchLevel(ln);
        if (!level->checkAllocated(d_U_scratch_idx)) level->allocatePatchData(d_U_scratch_idx);
        if (!level->checkAllocated(d_rho_interp_scratch_idx)) level->allocatePatchData(d_rho_interp_scratch_idx);
        if (!level->checkAllocated(d_rho_interp_new_idx)) level->allocatePatchData(d_rho_interp_new_idx);
    }

    // Fill ghost cell values for velocity
    static const bool homogeneous_bc = false;
    typedef HierarchyGhostCellInterpolation::InterpolationTransactionComponent InterpolationTransactionComponent;
    std::vector<InterpolationTransactionComponent> transaction_comps(1);
    transaction_comps[0] = InterpolationTransactionComponent(d_U_scratch_idx,
                                                             U_idx,
                                                             "CONSERVATIVE_LINEAR_REFINE",
                                                             false,
                                                             "CONSERVATIVE_COARSEN",
                                                             d_bdry_extrap_type,
                                                             false,
                                                             d_bc_coefs);
    d_hier_bdry_fill->resetTransactionComponents(transaction_comps);
    d_hier_bdry_fill->setHomogeneousBc(homogeneous_bc);
    StaggeredStokesPhysicalBoundaryHelper::setupBcCoefObjects(d_bc_coefs, NULL, d_U_scratch_idx, -1, homogeneous_bc);
    d_hier_bdry_fill->fillData(d_solution_time);
    StaggeredStokesPhysicalBoundaryHelper::resetBcCoefObjects(d_bc_coefs, NULL);
    d_hier_bdry_fill->resetTransactionComponents(d_transaction_comps);

    // Fill ghost cells for density
    InterpolationTransactionComponent rho_transaction(d_rho_interp_scratch_idx,
                                                      d_rho_interp_current_idx,
                                                      "CONSERVATIVE_LINEAR_REFINE",
                                                      false,
                                                      "CONSERVATIVE_COARSEN",
                                                      d_bdry_extrap_type,
                                                      false,
                                                      NULL);
    Pointer<HierarchyGhostCellInterpolation> hier_rho_bdry_fill = new HierarchyGhostCellInterpolation();
    hier_rho_bdry_fill->initializeOperatorState(rho_transaction, d_hierarchy);
    hier_rho_bdry_fill->fillData(d_solution_time);

    // d_hier_sc_data_ops->printData(d_rho_interp_scratch_idx, pout, false);

    // Compute the convective derivative.
    for (int ln = d_coarsest_ln; ln <= d_finest_ln; ++ln)
    {
        Pointer<PatchLevel<NDIM> > level = d_hierarchy->getPatchLevel(ln);
        for (PatchLevel<NDIM>::Iterator p(level); p; p++)
        {
            Pointer<Patch<NDIM> > patch = level->getPatch(p());

            const Pointer<CartesianPatchGeometry<NDIM> > patch_geom = patch->getPatchGeometry();
            const double* const dx = patch_geom->getDx();

            const Box<NDIM>& patch_box = patch->getBox();
            const IntVector<NDIM>& patch_lower = patch_box.lower();
            const IntVector<NDIM>& patch_upper = patch_box.upper();

            Pointer<SideData<NDIM, double> > N_data = patch->getPatchData(N_idx);
            Pointer<SideData<NDIM, double> > U_data = patch->getPatchData(d_U_scratch_idx);
            Pointer<SideData<NDIM, double> > R_data = patch->getPatchData(d_rho_interp_scratch_idx);
            Pointer<SideData<NDIM, double> > R_new_data = patch->getPatchData(d_rho_interp_new_idx);

            // Define variables that live on the "faces" of control volumes centered about side-centered staggered velocity components
            const IntVector<NDIM> ghosts = IntVector<NDIM>(1);
            boost::array<Box<NDIM>, NDIM> side_boxes;
            boost::array<Pointer<FaceData<NDIM, double> >, NDIM> U_adv_data;
            boost::array<Pointer<FaceData<NDIM, double> >, NDIM> U_half_data;
            boost::array<Pointer<FaceData<NDIM, double> >, NDIM> R_adv_data;
            boost::array<Pointer<FaceData<NDIM, double> >, NDIM> P_adv_data;
            for (unsigned int axis = 0; axis < NDIM; ++axis)
            {
                side_boxes[axis] = SideGeometry<NDIM>::toSideBox(patch_box, axis);
                U_adv_data[axis] = new FaceData<NDIM, double>(side_boxes[axis], 1, ghosts);
                U_half_data[axis] = new FaceData<NDIM, double>(side_boxes[axis], 1, ghosts);
                R_adv_data[axis] = new FaceData<NDIM, double>(side_boxes[axis], 1, ghosts);
                P_adv_data[axis] = new FaceData<NDIM, double>(side_boxes[axis], 1, ghosts);
            }
#if (NDIM == 2)
            NAVIER_STOKES_INTERP_COMPS_FC(patch_lower(0),
                                          patch_upper(0),
                                          patch_lower(1),
                                          patch_upper(1),
                                          U_data->getGhostCellWidth()(0),
                                          U_data->getGhostCellWidth()(1),
                                          U_data->getPointer(0),
                                          U_data->getPointer(1),
                                          side_boxes[0].lower(0),
                                          side_boxes[0].upper(0),
                                          side_boxes[0].lower(1),
                                          side_boxes[0].upper(1),
                                          U_adv_data[0]->getGhostCellWidth()(0),
                                          U_adv_data[0]->getGhostCellWidth()(1),
                                          U_adv_data[0]->getPointer(0),
                                          U_adv_data[0]->getPointer(1),
                                          side_boxes[1].lower(0),
                                          side_boxes[1].upper(0),
                                          side_boxes[1].lower(1),
                                          side_boxes[1].upper(1),
                                          U_adv_data[1]->getGhostCellWidth()(0),
                                          U_adv_data[1]->getGhostCellWidth()(1),
                                          U_adv_data[1]->getPointer(0),
                                          U_adv_data[1]->getPointer(1));

            VC_NAVIER_STOKES_UPWIND_DENSITY_FC(patch_lower(0),
                                               patch_upper(0),
                                               patch_lower(1),
                                               patch_upper(1),
                                               R_data->getGhostCellWidth()(0),
                                               R_data->getGhostCellWidth()(1),
                                               R_data->getPointer(0),
                                               R_data->getPointer(1),
                                               side_boxes[0].lower(0),
                                               side_boxes[0].upper(0),
                                               side_boxes[0].lower(1),
                                               side_boxes[0].upper(1),
                                               U_adv_data[0]->getGhostCellWidth()(0),
                                               U_adv_data[0]->getGhostCellWidth()(1),
                                               U_adv_data[0]->getPointer(0),
                                               U_adv_data[0]->getPointer(1),
                                               R_adv_data[0]->getGhostCellWidth()(0),
                                               R_adv_data[0]->getGhostCellWidth()(1),
                                               R_adv_data[0]->getPointer(0),
                                               R_adv_data[0]->getPointer(1),
                                               side_boxes[1].lower(0),
                                               side_boxes[1].upper(0),
                                               side_boxes[1].lower(1),
                                               side_boxes[1].upper(1),
                                               U_adv_data[1]->getGhostCellWidth()(0),
                                               U_adv_data[1]->getGhostCellWidth()(1),
                                               U_adv_data[1]->getPointer(0),
                                               U_adv_data[1]->getPointer(1),
                                               R_adv_data[1]->getGhostCellWidth()(0),
                                               R_adv_data[1]->getGhostCellWidth()(1),
                                               R_adv_data[1]->getPointer(0),
                                               R_adv_data[1]->getPointer(1));
#endif
#if (NDIM == 3)
            NAVIER_STOKES_INTERP_COMPS_FC(patch_lower(0),
                                          patch_upper(0),
                                          patch_lower(1),
                                          patch_upper(1),
                                          patch_lower(2),
                                          patch_upper(2),
                                          U_data->getGhostCellWidth()(0),
                                          U_data->getGhostCellWidth()(1),
                                          U_data->getGhostCellWidth()(2),
                                          U_data->getPointer(0),
                                          U_data->getPointer(1),
                                          U_data->getPointer(2),
                                          side_boxes[0].lower(0),
                                          side_boxes[0].upper(0),
                                          side_boxes[0].lower(1),
                                          side_boxes[0].upper(1),
                                          side_boxes[0].lower(2),
                                          side_boxes[0].upper(2),
                                          U_adv_data[0]->getGhostCellWidth()(0),
                                          U_adv_data[0]->getGhostCellWidth()(1),
                                          U_adv_data[0]->getGhostCellWidth()(2),
                                          U_adv_data[0]->getPointer(0),
                                          U_adv_data[0]->getPointer(1),
                                          U_adv_data[0]->getPointer(2),
                                          side_boxes[1].lower(0),
                                          side_boxes[1].upper(0),
                                          side_boxes[1].lower(1),
                                          side_boxes[1].upper(1),
                                          side_boxes[1].lower(2),
                                          side_boxes[1].upper(2),
                                          U_adv_data[1]->getGhostCellWidth()(0),
                                          U_adv_data[1]->getGhostCellWidth()(1),
                                          U_adv_data[1]->getGhostCellWidth()(2),
                                          U_adv_data[1]->getPointer(0),
                                          U_adv_data[1]->getPointer(1),
                                          U_adv_data[1]->getPointer(2),
                                          side_boxes[2].lower(0),
                                          side_boxes[2].upper(0),
                                          side_boxes[2].lower(1),
                                          side_boxes[2].upper(1),
                                          side_boxes[2].lower(2),
                                          side_boxes[2].upper(2),
                                          U_adv_data[2]->getGhostCellWidth()(0),
                                          U_adv_data[2]->getGhostCellWidth()(1),
                                          U_adv_data[2]->getGhostCellWidth()(2),
                                          U_adv_data[2]->getPointer(0),
                                          U_adv_data[2]->getPointer(1),
                                          U_adv_data[2]->getPointer(2));

            VC_NAVIER_STOKES_UPWIND_DENSITY_FC(patch_lower(0),
                                               patch_upper(0),
                                               patch_lower(1),
                                               patch_upper(1),
                                               patch_lower(2),
                                               patch_upper(2),
                                               R_data->getGhostCellWidth()(0),
                                               R_data->getGhostCellWidth()(1),
                                               R_data->getGhostCellWidth()(2),
                                               R_data->getPointer(0),
                                               R_data->getPointer(1),
                                               R_data->getPointer(2),
                                               side_boxes[0].lower(0),
                                               side_boxes[0].upper(0),
                                               side_boxes[0].lower(1),
                                               side_boxes[0].upper(1),
                                               side_boxes[0].lower(2),
                                               side_boxes[0].upper(2),
                                               U_adv_data[0]->getGhostCellWidth()(0),
                                               U_adv_data[0]->getGhostCellWidth()(1),
                                               U_adv_data[0]->getGhostCellWidth()(2),
                                               U_adv_data[0]->getPointer(0),
                                               U_adv_data[0]->getPointer(1),
                                               U_adv_data[0]->getPointer(2),
                                               R_adv_data[0]->getGhostCellWidth()(0),
                                               R_adv_data[0]->getGhostCellWidth()(1),
                                               R_adv_data[0]->getGhostCellWidth()(2),
                                               R_adv_data[0]->getPointer(0),
                                               R_adv_data[0]->getPointer(1),
                                               R_adv_data[0]->getPointer(2),
                                               side_boxes[1].lower(0),
                                               side_boxes[1].upper(0),
                                               side_boxes[1].lower(1),
                                               side_boxes[1].upper(1),
                                               side_boxes[1].lower(2),
                                               side_boxes[1].upper(2),
                                               U_adv_data[1]->getGhostCellWidth()(0),
                                               U_adv_data[1]->getGhostCellWidth()(1),
                                               U_adv_data[1]->getGhostCellWidth()(2),
                                               U_adv_data[1]->getPointer(0),
                                               U_adv_data[1]->getPointer(1),
                                               U_adv_data[1]->getPointer(2),
                                               R_adv_data[1]->getGhostCellWidth()(0),
                                               R_adv_data[1]->getGhostCellWidth()(1),
                                               R_adv_data[1]->getGhostCellWidth()(2),
                                               R_adv_data[1]->getPointer(0),
                                               R_adv_data[1]->getPointer(1),
                                               R_adv_data[1]->getPointer(2),
                                               side_boxes[2].lower(0),
                                               side_boxes[2].upper(0),
                                               side_boxes[2].lower(1),
                                               side_boxes[2].upper(1),
                                               side_boxes[2].lower(2),
                                               side_boxes[2].upper(2),
                                               U_adv_data[2]->getGhostCellWidth()(0),
                                               U_adv_data[2]->getGhostCellWidth()(1),
                                               U_adv_data[2]->getGhostCellWidth()(2),
                                               U_adv_data[2]->getPointer(0),
                                               U_adv_data[2]->getPointer(1),
                                               U_adv_data[2]->getPointer(2),
                                               R_adv_data[2]->getGhostCellWidth()(0),
                                               R_adv_data[2]->getGhostCellWidth()(1),
                                               R_adv_data[2]->getGhostCellWidth()(2),
                                               R_adv_data[2]->getPointer(0),
                                               R_adv_data[2]->getPointer(1),
                                               R_adv_data[2]->getPointer(2));
#endif
            for (unsigned int axis = 0; axis < NDIM; ++axis)
            {
                Pointer<SideData<NDIM, double> > dU_data =
                    new SideData<NDIM, double>(U_data->getBox(), U_data->getDepth(), U_data->getGhostCellWidth());
                Pointer<SideData<NDIM, double> > U_L_data =
                    new SideData<NDIM, double>(U_data->getBox(), U_data->getDepth(), U_data->getGhostCellWidth());
                Pointer<SideData<NDIM, double> > U_R_data =
                    new SideData<NDIM, double>(U_data->getBox(), U_data->getDepth(), U_data->getGhostCellWidth());
                Pointer<SideData<NDIM, double> > U_scratch1_data =
                    new SideData<NDIM, double>(U_data->getBox(), U_data->getDepth(), U_data->getGhostCellWidth());
#if (NDIM == 3)
                Pointer<SideData<NDIM, double> > U_scratch2_data =
                    new SideData<NDIM, double>(U_data->getBox(), U_data->getDepth(), U_data->getGhostCellWidth());
#endif
#if (NDIM == 2)
                GODUNOV_EXTRAPOLATE_FC(side_boxes[axis].lower(0),
                                       side_boxes[axis].upper(0),
                                       side_boxes[axis].lower(1),
                                       side_boxes[axis].upper(1),
                                       U_data->getGhostCellWidth()(0),
                                       U_data->getGhostCellWidth()(1),
                                       U_data->getPointer(axis),
                                       U_scratch1_data->getPointer(axis),
                                       dU_data->getPointer(axis),
                                       U_L_data->getPointer(axis),
                                       U_R_data->getPointer(axis),
                                       U_adv_data[axis]->getGhostCellWidth()(0),
                                       U_adv_data[axis]->getGhostCellWidth()(1),
                                       U_half_data[axis]->getGhostCellWidth()(0),
                                       U_half_data[axis]->getGhostCellWidth()(1),
                                       U_adv_data[axis]->getPointer(0),
                                       U_adv_data[axis]->getPointer(1),
                                       U_half_data[axis]->getPointer(0),
                                       U_half_data[axis]->getPointer(1));
#endif
#if (NDIM == 3)
                GODUNOV_EXTRAPOLATE_FC(side_boxes[axis].lower(0),
                                       side_boxes[axis].upper(0),
                                       side_boxes[axis].lower(1),
                                       side_boxes[axis].upper(1),
                                       side_boxes[axis].lower(2),
                                       side_boxes[axis].upper(2),
                                       U_data->getGhostCellWidth()(0),
                                       U_data->getGhostCellWidth()(1),
                                       U_data->getGhostCellWidth()(2),
                                       U_data->getPointer(axis),
                                       U_scratch1_data->getPointer(axis),
                                       U_scratch2_data->getPointer(axis),
                                       dU_data->getPointer(axis),
                                       U_L_data->getPointer(axis),
                                       U_R_data->getPointer(axis),
                                       U_adv_data[axis]->getGhostCellWidth()(0),
                                       U_adv_data[axis]->getGhostCellWidth()(1),
                                       U_adv_data[axis]->getGhostCellWidth()(2),
                                       U_half_data[axis]->getGhostCellWidth()(0),
                                       U_half_data[axis]->getGhostCellWidth()(1),
                                       U_half_data[axis]->getGhostCellWidth()(2),
                                       U_adv_data[axis]->getPointer(0),
                                       U_adv_data[axis]->getPointer(1),
                                       U_adv_data[axis]->getPointer(2),
                                       U_half_data[axis]->getPointer(0),
                                       U_half_data[axis]->getPointer(1),
                                       U_half_data[axis]->getPointer(2));
#endif
            }
#if (NDIM == 2)
            VC_NAVIER_STOKES_RESET_ADV_MOMENTUM_FC(side_boxes[0].lower(0),
                                                   side_boxes[0].upper(0),
                                                   side_boxes[0].lower(1),
                                                   side_boxes[0].upper(1),
                                                   P_adv_data[0]->getGhostCellWidth()(0),
                                                   P_adv_data[0]->getGhostCellWidth()(1),
                                                   P_adv_data[0]->getPointer(0),
                                                   P_adv_data[0]->getPointer(1),
                                                   R_adv_data[0]->getGhostCellWidth()(0),
                                                   R_adv_data[0]->getGhostCellWidth()(1),
                                                   R_adv_data[0]->getPointer(0),
                                                   R_adv_data[0]->getPointer(1),
                                                   U_half_data[0]->getGhostCellWidth()(0),
                                                   U_half_data[0]->getGhostCellWidth()(1),
                                                   U_half_data[0]->getPointer(0),
                                                   U_half_data[0]->getPointer(1),
                                                   side_boxes[1].lower(0),
                                                   side_boxes[1].upper(0),
                                                   side_boxes[1].lower(1),
                                                   side_boxes[1].upper(1),
                                                   P_adv_data[1]->getGhostCellWidth()(0),
                                                   P_adv_data[1]->getGhostCellWidth()(1),
                                                   P_adv_data[1]->getPointer(0),
                                                   P_adv_data[1]->getPointer(1),
                                                   R_adv_data[1]->getGhostCellWidth()(0),
                                                   R_adv_data[1]->getGhostCellWidth()(1),
                                                   R_adv_data[1]->getPointer(0),
                                                   R_adv_data[1]->getPointer(1),
                                                   U_half_data[1]->getGhostCellWidth()(0),
                                                   U_half_data[1]->getGhostCellWidth()(1),
                                                   U_half_data[1]->getPointer(0),
                                                   U_half_data[1]->getPointer(1));
#endif
#if (NDIM == 3)
            VC_NAVIER_STOKES_RESET_ADV_MOMENTUM_FC(side_boxes[0].lower(0),
                                                   side_boxes[0].upper(0),
                                                   side_boxes[0].lower(1),
                                                   side_boxes[0].upper(1),
                                                   side_boxes[0].lower(2),
                                                   side_boxes[0].upper(2),
                                                   P_adv_data[0]->getGhostCellWidth()(0),
                                                   P_adv_data[0]->getGhostCellWidth()(1),
                                                   P_adv_data[0]->getGhostCellWidth()(2),
                                                   P_adv_data[0]->getPointer(0),
                                                   P_adv_data[0]->getPointer(1),
                                                   P_adv_data[0]->getPointer(2),
                                                   R_adv_data[0]->getGhostCellWidth()(0),
                                                   R_adv_data[0]->getGhostCellWidth()(1),
                                                   R_adv_data[0]->getGhostCellWidth()(2),
                                                   R_adv_data[0]->getPointer(0),
                                                   R_adv_data[0]->getPointer(1),
                                                   R_adv_data[0]->getPointer(2),
                                                   U_half_data[0]->getGhostCellWidth()(0),
                                                   U_half_data[0]->getGhostCellWidth()(1),
                                                   U_half_data[0]->getGhostCellWidth()(2),
                                                   U_half_data[0]->getPointer(0),
                                                   U_half_data[0]->getPointer(1),
                                                   U_half_data[0]->getPointer(2),
                                                   side_boxes[1].lower(0),
                                                   side_boxes[1].upper(0),
                                                   side_boxes[1].lower(1),
                                                   side_boxes[1].upper(1),
                                                   side_boxes[1].lower(2),
                                                   side_boxes[1].upper(2),
                                                   P_adv_data[1]->getGhostCellWidth()(0),
                                                   P_adv_data[1]->getGhostCellWidth()(1),
                                                   P_adv_data[1]->getGhostCellWidth()(2),
                                                   P_adv_data[1]->getPointer(0),
                                                   P_adv_data[1]->getPointer(1),
                                                   P_adv_data[1]->getPointer(2),
                                                   R_adv_data[1]->getGhostCellWidth()(0),
                                                   R_adv_data[1]->getGhostCellWidth()(1),
                                                   R_adv_data[1]->getGhostCellWidth()(2),
                                                   R_adv_data[1]->getPointer(0),
                                                   R_adv_data[1]->getPointer(1),
                                                   R_adv_data[1]->getPointer(2),
                                                   U_half_data[1]->getGhostCellWidth()(0),
                                                   U_half_data[1]->getGhostCellWidth()(1),
                                                   U_half_data[1]->getGhostCellWidth()(2),
                                                   U_half_data[1]->getPointer(0),
                                                   U_half_data[1]->getPointer(1),
                                                   U_half_data[1]->getPointer(2),
                                                   side_boxes[2].lower(0),
                                                   side_boxes[2].upper(0),
                                                   side_boxes[2].lower(1),
                                                   side_boxes[2].upper(1),
                                                   side_boxes[2].lower(2),
                                                   side_boxes[2].upper(2),
                                                   P_adv_data[2]->getGhostCellWidth()(0),
                                                   P_adv_data[2]->getGhostCellWidth()(1),
                                                   P_adv_data[2]->getGhostCellWidth()(2),
                                                   P_adv_data[2]->getPointer(0),
                                                   P_adv_data[2]->getPointer(1),
                                                   P_adv_data[2]->getPointer(2),
                                                   R_adv_data[2]->getGhostCellWidth()(0),
                                                   R_adv_data[2]->getGhostCellWidth()(1),
                                                   R_adv_data[2]->getGhostCellWidth()(2),
                                                   R_adv_data[2]->getPointer(0),
                                                   R_adv_data[2]->getPointer(1),
                                                   R_adv_data[2]->getPointer(2),
                                                   U_half_data[2]->getGhostCellWidth()(0),
                                                   U_half_data[2]->getGhostCellWidth()(1),
                                                   U_half_data[2]->getGhostCellWidth()(2),
                                                   U_half_data[2]->getPointer(0),
                                                   U_half_data[2]->getPointer(1),
                                                   U_half_data[2]->getPointer(2));
  #endif

            for (unsigned int axis = 0; axis < NDIM; ++axis)
            {
                switch (d_difference_form)
                {
                case CONSERVATIVE:
#if (NDIM == 2)
                    CONVECT_DERIVATIVE_FC(dx,
                                          side_boxes[axis].lower(0),
                                          side_boxes[axis].upper(0),
                                          side_boxes[axis].lower(1),
                                          side_boxes[axis].upper(1),
                                          P_adv_data[axis]->getGhostCellWidth()(0),
                                          P_adv_data[axis]->getGhostCellWidth()(1),
                                          U_half_data[axis]->getGhostCellWidth()(0),
                                          U_half_data[axis]->getGhostCellWidth()(1),
                                          P_adv_data[axis]->getPointer(0),
                                          P_adv_data[axis]->getPointer(1),
                                          U_half_data[axis]->getPointer(0),
                                          U_half_data[axis]->getPointer(1),
                                          N_data->getGhostCellWidth()(0),
                                          N_data->getGhostCellWidth()(1),
                                          N_data->getPointer(axis));
#endif
#if (NDIM == 3)
                    CONVECT_DERIVATIVE_FC(dx,
                                          side_boxes[axis].lower(0),
                                          side_boxes[axis].upper(0),
                                          side_boxes[axis].lower(1),
                                          side_boxes[axis].upper(1),
                                          side_boxes[axis].lower(2),
                                          side_boxes[axis].upper(2),
                                          P_adv_data[axis]->getGhostCellWidth()(0),
                                          P_adv_data[axis]->getGhostCellWidth()(1),
                                          P_adv_data[axis]->getGhostCellWidth()(2),
                                          U_half_data[axis]->getGhostCellWidth()(0),
                                          U_half_data[axis]->getGhostCellWidth()(1),
                                          U_half_data[axis]->getGhostCellWidth()(2),
                                          P_adv_data[axis]->getPointer(0),
                                          P_adv_data[axis]->getPointer(1),
                                          P_adv_data[axis]->getPointer(2),
                                          U_half_data[axis]->getPointer(0),
                                          U_half_data[axis]->getPointer(1),
                                          U_half_data[axis]->getPointer(2),
                                          N_data->getGhostCellWidth()(0),
                                          N_data->getGhostCellWidth()(1),
                                          N_data->getGhostCellWidth()(2),
                                          N_data->getPointer(axis));
#endif
                    break;
                default:
                    TBOX_ERROR("VCINSStaggeredConservativePPMConvectiveOperator::applyConvectiveOperator():\n"
                               << "  unsupported differencing form: "
                               << enum_to_string<ConvectiveDifferencingType>(d_difference_form)
                               << " \n"
                               << "  valid choices are: CONSERVATIVE\n");
                }
            }

            // Finally, compute an updated side-centered rho quantity rho^{n+1} = rho^n - dt*div(rho_adv*u_adv)
            for (unsigned int axis = 0; axis < NDIM; ++axis)
            {
#if (NDIM == 2)
                VC_UPDATE_DENSITY_FC(dx,
                                     d_dt,
                                     side_boxes[axis].lower(0),
                                     side_boxes[axis].upper(0),
                                     side_boxes[axis].lower(1),
                                     side_boxes[axis].upper(1),
                                     P_adv_data[axis]->getGhostCellWidth()(0),
                                     P_adv_data[axis]->getGhostCellWidth()(1),
                                     P_adv_data[axis]->getPointer(0),
                                     P_adv_data[axis]->getPointer(1),
                                     R_data->getGhostCellWidth()(0),
                                     R_data->getGhostCellWidth()(1),
                                     R_data->getPointer(axis),
                                     R_new_data->getGhostCellWidth()(0),
                                     R_new_data->getGhostCellWidth()(1),
                                     R_new_data->getPointer(axis));
#endif
#if (NDIM == 3)
                VC_UPDATE_DENSITY_FC(dx,
                                     d_dt,
                                     side_boxes[axis].lower(0),
                                     side_boxes[axis].upper(0),
                                     side_boxes[axis].lower(1),
                                     side_boxes[axis].upper(1),
                                     side_boxes[axis].lower(2),
                                     side_boxes[axis].upper(2),
                                     P_adv_data[axis]->getGhostCellWidth()(0),
                                     P_adv_data[axis]->getGhostCellWidth()(1),
                                     P_adv_data[axis]->getGhostCellWidth()(2),
                                     P_adv_data[axis]->getPointer(0),
                                     P_adv_data[axis]->getPointer(1),
                                     P_adv_data[axis]->getPointer(2),
                                     R_data->getGhostCellWidth()(0),
                                     R_data->getGhostCellWidth()(1),
                                     R_data->getGhostCellWidth()(2),
                                     R_data->getPointer(axis),
                                     R_new_data->getGhostCellWidth()(0),
                                     R_new_data->getGhostCellWidth()(1),
                                     R_new_data->getGhostCellWidth()(2),
                                     R_new_data->getPointer(axis));
#endif
            }
        }
    }

    // Deallocate scratch data.
    for (int ln = d_coarsest_ln; ln <= d_finest_ln; ++ln)
    {
        Pointer<PatchLevel<NDIM> > level = d_hierarchy->getPatchLevel(ln);
        if (level->checkAllocated(d_U_scratch_idx)) level->deallocatePatchData(d_U_scratch_idx);
        if (level->checkAllocated(d_rho_interp_scratch_idx)) level->deallocatePatchData(d_rho_interp_scratch_idx);
    }

    // Reset select options
    d_dt = -1.0;
    d_dt_is_set = false;
    d_rho_interp_current_idx = -1;
    d_rho_is_set = false;

    IBAMR_TIMER_STOP(t_apply_convective_operator);
    return;
} // applyConvectiveOperator

void
VCINSStaggeredConservativePPMConvectiveOperator::initializeOperatorState(const SAMRAIVectorReal<NDIM, double>& in,
                                                                         const SAMRAIVectorReal<NDIM, double>& out)
{
    IBAMR_TIMER_START(t_initialize_operator_state);

    if (d_is_initialized) deallocateOperatorState();

    // Get the hierarchy configuration.
    d_hierarchy = in.getPatchHierarchy();
    d_coarsest_ln = in.getCoarsestLevelNumber();
    d_finest_ln = in.getFinestLevelNumber();
#if !defined(NDEBUG)
    TBOX_ASSERT(d_hierarchy == out.getPatchHierarchy());
    TBOX_ASSERT(d_coarsest_ln == out.getCoarsestLevelNumber());
    TBOX_ASSERT(d_finest_ln == out.getFinestLevelNumber());
#else
    NULL_USE(out);
#endif

    // Setup the interpolation transaction information.
    typedef HierarchyGhostCellInterpolation::InterpolationTransactionComponent InterpolationTransactionComponent;
    d_transaction_comps.resize(1);
    d_transaction_comps[0] = InterpolationTransactionComponent(d_U_scratch_idx,
                                                               in.getComponentDescriptorIndex(0),
                                                               "CONSERVATIVE_LINEAR_REFINE",
                                                               false,
                                                               "CONSERVATIVE_COARSEN",
                                                               d_bdry_extrap_type,
                                                               false,
                                                               d_bc_coefs);

    // Initialize the interpolation operators.
    d_hier_bdry_fill = new HierarchyGhostCellInterpolation();
    d_hier_bdry_fill->initializeOperatorState(d_transaction_comps, d_hierarchy);

    // Initialize the BC helper.
    d_bc_helper = new StaggeredStokesPhysicalBoundaryHelper();
    d_bc_helper->cacheBcCoefData(d_bc_coefs, d_solution_time, d_hierarchy);

    d_is_initialized = true;

    IBAMR_TIMER_STOP(t_initialize_operator_state);
    return;
} // initializeOperatorState

void
VCINSStaggeredConservativePPMConvectiveOperator::deallocateOperatorState()
{
    if (!d_is_initialized) return;

    IBAMR_TIMER_START(t_deallocate_operator_state);

    // Deallocate the communications operators and BC helpers.
    d_hier_bdry_fill.setNull();
    d_bc_helper.setNull();

    d_is_initialized = false;

    IBAMR_TIMER_STOP(t_deallocate_operator_state);
    return;
} // deallocateOperatorState

void
VCINSStaggeredConservativePPMConvectiveOperator::setInterpolatedDensityPatchDataIndex(int rho_interp_idx)
{
#if !defined(NDEBUG)
    TBOX_ASSERT(rho_interp_idx >= 0);
#endif
    d_rho_is_set = true;
    d_rho_interp_current_idx = rho_interp_idx;
} // setInterpolatedDensityPatchDataIndex

void
VCINSStaggeredConservativePPMConvectiveOperator::setTimeStepSize(double dt)
{
#if !defined(NDEBUG)
    TBOX_ASSERT(dt >= 0.0);
#endif
    d_dt_is_set = true;
    d_dt = dt;
} // setTimeStepSize

int
VCINSStaggeredConservativePPMConvectiveOperator::getUpdatedInterpolatedDensityPatchDataIndex()
{
#if !defined(NDEBUG)
    TBOX_ASSERT(d_rho_interp_new_idx >= 0);
#endif
    return d_rho_interp_new_idx;
} // getUpdatedInterpolatedDensityPatchDataIndex

/////////////////////////////// PROTECTED ////////////////////////////////////

/////////////////////////////// PRIVATE //////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

} // namespace IBAMR

//////////////////////////////////////////////////////////////////////////////
