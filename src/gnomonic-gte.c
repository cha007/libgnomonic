/*
 * libgnomonic - Gnomonic projection algorithms library
 *
 * Copyright (c) 2013-2014 FOXEL SA - http://foxel.ch
 * Please read <http://foxel.ch/license> for more information.
 *
 *
 * Author(s):
 *
 *      Nils Hamel <n.hamel@foxel.ch>
 *
 *
 * This file is part of the FOXEL project <http://foxel.ch>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Additional Terms:
 *
 *      You are required to preserve legal notices and author attributions in
 *      that material or in the Appropriate Legal Notices displayed by works
 *      containing it.
 *
 *      You are required to attribute the work as explained in the "Usage and
 *      Attribution" section of <http://foxel.ch/license>.
 */


/*
    Source - Includes
 */

    # include "gnomonic-gte.h"

/*
    Source - Reprojection of gnomonic image back into equirectangluar image
 */

    void gnomonic_gte(

        unsigned char * pan_img,
        long int pan_w,
        long int pan_h,
        long int pan_d,
        unsigned char * rct_img,
        long int rct_w,
        long int rct_h,
        long int rct_d,
        double nad_t,
        double nad_p,
        double app_t,
        double app_p,
        interp  int_m

    ) {

        /* Projection variables */
        static long int rx = 0;
        static long int ry = 0;
        static double ux = 0.0;
        static double uy = 0.0;
        static double pt = 0.0;
        static double pp = 0.0;
        static double at = 0.0;
        static double ap = 0.0;

        /* Position vector */
        static double pv[3] = { 0.0, 0.0, 0.0 };
        static double kv[3] = { 0.0, 0.0, 0.0 };

        /* Rotation matrix */
        static double Ry[3][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 0.0 } };
        static double Rz[3][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 } };

        /* Rotation matrix - y */
        Ry[0][0] = + cos( - nad_p );
        Ry[0][2] = + sin( - nad_p );
        Ry[2][0] = - Ry[0][2];
        Ry[2][2] = + Ry[0][0];

        /* Rotation matrix - z */
        Rz[0][0] = + cos( - nad_t );
        Rz[0][1] = - sin( - nad_t );
        Rz[1][0] = - Rz[0][1];
        Rz[1][1] = + Rz[0][0];

        /* Angular apperture tangent computation */
        at = tan( app_t );
        ap = tan( app_p );

        /* Planar projection referential y-loop */
        for ( ry = 0; ry < pan_h; ry++ ) {

            /* Planar projection referential x-loop */
            for ( rx = 0; rx < pan_w; rx++ ) {

                /* Build angular position */
                pt = ( ( double ) rx / ( double ) ( pan_w - 1 ) ) * 2.0 * GNOMONIC_PI;
                pp = ( ( ( double ) ry / ( double ) ( pan_h - 1 ) ) - 0.5 ) * GNOMONIC_PI;

                /* Build position vector */
                pv[0] = cos( pt ) * cos( pp );
                pv[1] = sin( pt ) * cos( pp );
                pv[2] = sin( pp );

                /* Apply rotation due to t-nadir and p-nadir */
                kv[0] = Rz[0][0] * pv[0] + Rz[0][1] * pv[1];
                kv[1] = Rz[1][0] * pv[0] + Rz[1][1] * pv[1];
                kv[2] = pv[2];
                pv[0] = Ry[0][0] * kv[0] + Ry[0][2] * kv[2];
                pv[1] = kv[1];
                pv[2] = Ry[2][0] * kv[0] + Ry[2][2] * kv[2];

                /* Verify projection domain */
                if ( pv[0] > 0 ) {

                    /* Apply projection and scaling */
                    ux = ( ( ( pv[1] / pv[0] / at ) + 1.0 ) / 2.0 ) * ( rct_w - 1 );
                    uy = ( ( ( pv[2] / pv[0] / ap ) + 1.0 ) / 2.0 ) * ( rct_h - 1 );

                    /* Verify gnomonic domain */
                    if ( ( ux >= 0 ) && ( ux < rct_w ) && ( uy >= 0 ) && ( uy < rct_h ) ) {

                        /* Interpolation process */
                        * ( pan_img + pan_d * ( pan_w * ry + rx )     ) = int_m( rct_img, rct_w, rct_h, rct_d, 0, ux, uy );
                        * ( pan_img + pan_d * ( pan_w * ry + rx ) + 1 ) = int_m( rct_img, rct_w, rct_h, rct_d, 1, ux, uy );
                        * ( pan_img + pan_d * ( pan_w * ry + rx ) + 2 ) = int_m( rct_img, rct_w, rct_h, rct_d, 2, ux, uy );

                    }

                }

            }

        }

    }

/*
    Source - Blended reprojection of gnomonic image back into equirectangluar image
 */

    void gnomonic_gte_blend(

        unsigned char * pan_img,
        long int pan_w,
        long int pan_h,
        long int pan_d,
        unsigned char * rct_img,
        unsigned char * mas_img,
        long int rct_w,
        long int rct_h,
        long int rct_d,
        double nad_t,
        double nad_p,
        double app_t,
        double app_p,
        interp  int_m

    ) {

        /* Projection variables */
        static long int rx = 0;
        static long int ry = 0;
        static double ux = 0.0;
        static double uy = 0.0;
        static double bf = 0.0;
        static double pt = 0.0;
        static double pp = 0.0;
        static double at = 0.0;
        static double ap = 0.0;

        /* Position vector */
        static double pv[3] = { 0.0, 0.0, 0.0 };
        static double kv[3] = { 0.0, 0.0, 0.0 };

        /* Rotation matrix */
        static double Ry[3][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 0.0 } };
        static double Rz[3][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 } };

        /* Rotation matrix - y */
        Ry[0][0] = + cos( - nad_p );
        Ry[0][2] = + sin( - nad_p );
        Ry[2][0] = - Ry[0][2];
        Ry[2][2] = + Ry[0][0];

        /* Rotation matrix - z */
        Rz[0][0] = + cos( - nad_t );
        Rz[0][1] = - sin( - nad_t );
        Rz[1][0] = - Rz[0][1];
        Rz[1][1] = + Rz[0][0];

        /* Angular apperture tangent computation */
        at = tan( app_t );
        ap = tan( app_p );

        /* Planar projection referential y-loop */
        for ( ry = 0; ry < pan_h; ry++ ) {

            /* Planar projection referential x-loop */
            for ( rx = 0; rx < pan_w; rx++ ) {

                /* Build angular position */
                pt = ( ( double ) rx / ( double ) ( pan_w - 1 ) ) * 2.0 * GNOMONIC_PI;
                pp = ( ( ( double ) ry / ( double ) ( pan_h - 1 ) ) - 0.5 ) * GNOMONIC_PI;

                /* Build position vector */
                pv[0] = cos( pt ) * cos( pp );
                pv[1] = sin( pt ) * cos( pp );
                pv[2] = sin( pp );

                /* Apply rotation due to t-nadir and p-nadir */
                kv[0] = Rz[0][0] * pv[0] + Rz[0][1] * pv[1];
                kv[1] = Rz[1][0] * pv[0] + Rz[1][1] * pv[1];
                kv[2] = pv[2];
                pv[0] = Ry[0][0] * kv[0] + Ry[0][2] * kv[2];
                pv[1] = kv[1];
                pv[2] = Ry[2][0] * kv[0] + Ry[2][2] * kv[2];

                /* Verify projection domain */
                if ( pv[0] > 0 ) {

                    /* Apply projection and scaling */
                    ux = ( ( ( pv[1] / pv[0] / at ) + 1.0 ) / 2.0 ) * ( rct_w - 1 );
                    uy = ( ( ( pv[2] / pv[0] / ap ) + 1.0 ) / 2.0 ) * ( rct_h - 1 );

                    /* Verify gnomonic domain */
                    if ( ( ux >= 0 ) && ( ux < rct_w ) && ( uy >= 0 ) && ( uy < rct_h ) ) {

                        /* Compute the blending factor */
                        bf = ( double ) * ( mas_img + rct_w * ( long int ) trunc( uy ) + ( long int ) trunc( ux ) ) / 255.0;

                        /* Interpolation and blending process */
                        * ( pan_img + pan_d * ( pan_w * ry + rx )     ) =
                        * ( pan_img + pan_d * ( pan_w * ry + rx )     ) * ( 1.0 - bf ) +
                        int_m( rct_img, rct_w, rct_h, rct_d, 0, ux, uy ) * bf;
                        * ( pan_img + pan_d * ( pan_w * ry + rx ) + 1 ) =
                        * ( pan_img + pan_d * ( pan_w * ry + rx ) + 1 ) * ( 1.0 - bf ) +
                        int_m( rct_img, rct_w, rct_h, rct_d, 1, ux, uy ) * bf;
                        * ( pan_img + pan_d * ( pan_w * ry + rx ) + 2 ) = * ( pan_img + pan_d * ( pan_w * ry + rx ) + 2 ) * ( 1.0 - bf ) +
                        int_m( rct_img, rct_w, rct_h, rct_d, 2, ux, uy ) * bf;

                    }

                }

            }

        }

    }
