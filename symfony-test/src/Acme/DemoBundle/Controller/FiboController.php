<?php

namespace Acme\DemoBundle\Controller;

use Symfony\Bundle\FrameworkBundle\Controller\Controller;
use Symfony\Component\HttpFoundation\RedirectResponse;
use Symfony\Component\HttpFoundation\Request;
use Acme\DemoBundle\Form\ContactType;

// these import the "@Route" and "@Template" annotations
use Sensio\Bundle\FrameworkExtraBundle\Configuration\Route;
use Sensio\Bundle\FrameworkExtraBundle\Configuration\Template;

class FiboController extends Controller
{
    /**
     * @Route("/{number}/{random}", name="fibo", defaults={"number" = 7, "random" = 1}))
     * @Template()
     */
    public function indexAction($number = 8, $random = 1 )
    {
        $newrelic_enabled = "no";
        if (function_exists("newrelic_start_transaction")) {
            $newrelic_enabled = "yes";
        }
        $version = "PHP: " . PHP_VERSION;
        if (defined("HHVM_VERSION")) {
            $version .= " HHVM: " . HHVM_VERSION;
        }

        return array('version' => $version, "newrelic" => $newrelic_enabled, 'number' => $number, 'result' => $this->fibonacci($number));
    }

    protected function fibonacci($n)
    {
        if ($n == 0) {
            return 0;
        }
        else if ($n == 1)
        {
            return 1;
        } else {
            return $this->fibonacci( $n - 1 ) + $this->fibonacci( $n - 2 );
        }
    }



}
